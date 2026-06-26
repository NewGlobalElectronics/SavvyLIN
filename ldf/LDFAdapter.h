#pragma once

// ============================================================
// LDFAdapter.h
// Couche d'adaptation LDF → remplace DBCHandler/DBC_* dans SavvyLIN
// Usage : #include "ldf/LDFAdapter.h"   (remplace dbchandler.h)
// ============================================================

#include <QString>
#include <QList>
#include <QDebug>
#include <map>

#include "LDFDatabaseManager.h"
#include "ICluster.h"
#include "../can_structs.h"
#include "ProtocolDefinitions/LINDefines.h"
// ─────────────────────────────────────────────────────────────
// Structures légères (remplacent DBC_SIGNAL / DBC_MESSAGE)
// ─────────────────────────────────────────────────────────────

struct LIN_SIGNAL
{
    QString      name;
    uint32_t     startBit      = 0;
    uint32_t     signalSize    = 1;   // en bits
    double       factor        = 1.0;
    double       bias          = 0.0;
    bool         intelByteOrder = true;
    bool         isSigned      = false;

    // Référence vers la frame parente (ne pas supprimer, appartient à LIN_FRAME)
    QString      parentFrameName;
    uint32_t     parentFrameID = 0;

    // Pointeur brut vers ISignal pour accès avancé (optionnel)
    ISignal*     rawSignal     = nullptr;

    // Calcule la valeur engineering depuis les octets bruts de la frame
    double getValue(const unsigned char* data, int dataLen) const
    {
        if (!rawSignal) return 0.0;
        unsigned __int64 rawVal = 0;
        rawSignal->GetRawValue((int)startBit, (int)signalSize,
                               dataLen, intelByteOrder, data, rawVal);
        double engVal = 0.0;
        rawSignal->GetEnggValueFromRaw(rawVal, engVal);
        return engVal;
    }
    bool processAsDouble(const CANFrame& frame, double& val) const
    {
        if (!rawSignal) return false;
        const unsigned char* data = reinterpret_cast<const unsigned char*>(frame.payload().constData());
        int dataLen = frame.payload().size();
        val = getValue(data, dataLen);
        return true;
    }

    // Remplace isSignalInMessage de DBC (LIN : toujours vrai si même ID)
    bool isSignalInMessage(const CANFrame& frame) const
    {
        return frame.frameId() == parentFrameID;
    }

    // Remplace getValueString de DBC (LIN : pas de value table standard, retourne false)
    bool getValueString(int64_t /*rawVal*/, QString& /*out*/) const
    {
        return false; // pas de value table dans LIN simple
    }
};

struct LIN_FRAME
{
    QString           name;
    uint32_t          ID     = 0;
    uint32_t          length = 0;          // en octets
    QList<LIN_SIGNAL*> signalList;

    ~LIN_FRAME()
    {
        qDeleteAll(signalList);
        signalList.clear();
    }

    LIN_SIGNAL* findSignalByName(const QString& sigName) const
    {
        for (LIN_SIGNAL* s : signalList)
            if (s->name == sigName) return s;
        return nullptr;
    }
};

// ─────────────────────────────────────────────────────────────
// LDFAdapter  —  singleton, remplace DBCHandler::getReference()
// ─────────────────────────────────────────────────────────────

class LDFAdapter
{
public:
    // ── Accès singleton ──────────────────────────────────────
    static LDFAdapter* getReference()
    {
        static LDFAdapter instance;
        return &instance;
    }

    // ── Recharge le cache depuis ICluster ────────────────────
    void refresh()
    {
        qDeleteAll(m_frames);
        m_frames.clear();

        ICluster* cluster = LDFDatabaseManager::GetDatabaseManager()->GetLDFCluster();
        if (!cluster) return;

        // ── Récupération des signaux (map uid → ISignal*) ────
        std::map<UID_ELEMENT, IElement*> sigElements;
        cluster->GetElementList(eSignalElement, sigElements);

        // ── Récupération des frames ──────────────────────────
        std::map<UID_ELEMENT, IElement*> frameElements;
        cluster->GetElementList(eFrameElement, frameElements);

        for (auto& pair : frameElements)
        {
            IFrame* pFrame = dynamic_cast<IFrame*>(pair.second);
            if (!pFrame) continue;

            // Filtre : seulement les Unconditional Frames
            FrameProps fProps;
            if (pFrame->GetProperties(fProps) != EC_SUCCESS) continue;
            // LinFrameProps est dans fProps si c'est un cluster LIN
            // On vérifie via GetFrameType
            eProtocolType pType;
            pFrame->GetFrameType(pType);

            LIN_FRAME* lf = new LIN_FRAME();

            std::string nameStr;
            pFrame->GetName(nameStr);
            lf->name = QString::fromStdString(nameStr);

            unsigned int fid = 0;
            pFrame->GetFrameId(fid);
            lf->ID = fid;

            unsigned int flen = 0;
            pFrame->GetLength(flen);
            lf->length = flen;

            // ── Signaux de la frame ──────────────────────────
            std::map<ISignal*, SignalInstanse> sigMap;
            if (pFrame->GetSignalList(sigMap) == EC_SUCCESS)
            {
                for (auto& sPair : sigMap)
                {
                    ISignal* pSig = sPair.first;
                    if (!pSig) continue;

                    SignalInstanse& inst = sPair.second;

                    LIN_SIGNAL* ls = new LIN_SIGNAL();

                    std::string sigName;
                    pSig->GetName(sigName);
                    ls->name = QString::fromStdString(sigName);

                    // Position dans la frame (depuis SignalInstanse)
                    //ls->startBit = inst.m_unStartBit;       // adapter si champ différent
                    ls->startBit = inst.m_nStartBit;
                    unsigned int sigLen = 0;
                    pSig->GetLength(sigLen);
                    ls->signalSize = sigLen;

                    ls->parentFrameName = lf->name;
                    ls->parentFrameID   = lf->ID;
                    ls->rawSignal       = pSig;

                    ls->startBit       = inst.m_nStartBit;
                    ls->intelByteOrder = (inst.m_ouSignalEndianess == eIntel);

                    LINSignalProps lsProps;
                    if (pSig->GetProperties(lsProps) == EC_SUCCESS)
                    {
                        ls->isSigned   = (lsProps.m_ouDataType == eSigned);
                        ls->signalSize = lsProps.m_unSignalSize;
                    }
                    // factor/bias : déjà gérés par ISignal::GetEnggValueFromRaw, rien à faire

                    lf->signalList.append(ls);
                }
            }

            m_frames.append(lf);
        }

        qDebug() << "[LDFAdapter] Chargé" << m_frames.count() << "frames LIN";
    }

    // ── Lookup par ID ─────────────────────────────────────────
    LIN_FRAME* findFrame(uint32_t id) const
    {
        for (LIN_FRAME* f : m_frames)
            if (f->ID == id) return f;
        return nullptr;
    }

    // ── Lookup par nom ────────────────────────────────────────
    LIN_FRAME* findFrame(const QString& name) const
    {
        for (LIN_FRAME* f : m_frames)
            if (f->name == name) return f;
        return nullptr;
    }

    // ── Lookup signal global par nom ─────────────────────────
    LIN_SIGNAL* findSignal(const QString& sigName) const
    {
        for (LIN_FRAME* f : m_frames)
        {
            LIN_SIGNAL* s = f->findSignalByName(sigName);
            if (s) return s;
        }
        return nullptr;
    }

    // ── Liste de toutes les frames ────────────────────────────
    const QList<LIN_FRAME*>& getAllFrames() const { return m_frames; }

    // ── Liste de tous les ECUs ────────────────────────────────
    QList<QString> getAllEcuNames() const
    {
        QList<QString> names;
        ICluster* cluster = LDFDatabaseManager::GetDatabaseManager()->GetLDFCluster();
        if (!cluster) return names;

        std::list<IEcu*> ecus;
        cluster->GetEcuList(ecus);
        for (IEcu* ecu : ecus)
        {
            std::string n;
            ecu->GetName(n);
            names.append(QString::fromStdString(n));
        }
        return names;
    }

    // ── Frames d'un ECU (par nom) ─────────────────────────────
    QList<LIN_FRAME*> getFramesForEcu(const QString& ecuName) const
    {
        QList<LIN_FRAME*> result;
        ICluster* cluster = LDFDatabaseManager::GetDatabaseManager()->GetLDFCluster();
        if (!cluster) return result;

        IEcu* pEcu = nullptr;
        cluster->GetEcu(ecuName.toStdString(), &pEcu);
        if (!pEcu) return result;

        std::list<IFrame*> frames;
        pEcu->GetFrameList(eTx, frames);   // TX frames de l'ECU
        for (IFrame* pf : frames)
        {
            unsigned int fid = 0;
            pf->GetFrameId(fid);
            LIN_FRAME* lf = findFrame(fid);
            if (lf) result.append(lf);
        }
        return result;
    }

    int getFrameCount() const { return m_frames.count(); }

private:
    LDFAdapter() = default;
    ~LDFAdapter() { qDeleteAll(m_frames); }
    LDFAdapter(const LDFAdapter&) = delete;
    LDFAdapter& operator=(const LDFAdapter&) = delete;

    QList<LIN_FRAME*> m_frames;
};

// ─────────────────────────────────────────────────────────────
// Alias de compatibilité  (pour minimiser les changements dans
// graphingwindow.cpp / signalviewerwindow.cpp)
// ─────────────────────────────────────────────────────────────

// Remplace DBC_SIGNAL*  → LIN_SIGNAL*
// Remplace DBC_MESSAGE* → LIN_FRAME*
// Remplace DBCHandler*  → LDFAdapter*
//
// Dans les .h qui déclarent ces types :
//   DBC_SIGNAL  → LIN_SIGNAL
//   DBC_MESSAGE → LIN_FRAME
//   DBCHandler  → LDFAdapter
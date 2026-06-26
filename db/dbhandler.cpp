
#include "dbhandler.h"

#include <QFile>
#include <QRegularExpression>
#include <QDebug>
#include <QMessageBox>
#include <QFileDialog>
#include <QApplication>
#include <QPalette>
#include <QSettings>
#include <QJsonDocument>
#include <QJsonArray>
#include <QJsonObject>
#include "utility.h"
#include "connections/canconmanager.h"

dbHandler* dbHandler::instance = nullptr;

// ============================================================
// parseLDFFile
// ============================================================
// ============================================================
// parseLDFFile  — version corrigée
// ============================================================
// ===========================================================
// dbhandler.cpp — section parseLDFFile + saveFile
// Tous les autres blocs (dbSignalHandler, dbMessageHandler,
// dbFile, dbHandler) restent identiques à l'original.
// ===========================================================

static bool parseLDFFile(const QString& filename, dbFile* file)
{
    qWarning() << "[LDF] Début du parsing du fichier:" << filename;

    QFile f(filename);
    if (!f.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "[LDF] ERREUR : impossible d'ouvrir le fichier" << filename;
        return false;
    }

    auto afterEq = [](const QString& line) -> QString {
        int idx = line.indexOf('=');
        return idx != -1 ? line.mid(idx + 1) : QString();
    };

    auto isSectionHeader = [](const QString& line, const QString& keyword) -> bool {
        QString s = line;
        s.remove('{').trimmed();
        return s.trimmed() == keyword;
    };

    db_NODE defaultNode;
    defaultNode.name    = "UNASSIGNED";
    defaultNode.comment = "Node not found in LDF";
    file->db_nodes.append(defaultNode);

    db_MESSAGE* currentMsg = nullptr;

    struct SigDef {
        int         size       = 8;
        double      initVal    = 0.0;
        QString     initValRaw;
        QString     publisher;
        QStringList subscribers;
    };
    QMap<QString, SigDef> sigDefs;

    // FIX 1 : SigEncoding supporte plusieurs physical_value ranges
    struct SigEncoding {
        QList<DB_SIGNAL::PhysRange> physRanges;
        QList<db_VAL_ENUM_ENTRY>    valList;
        bool                        hasPhysical = false;

        // Accesseurs de compatibilité — première plage
        double  factor() const { return physRanges.isEmpty() ? 1.0 : physRanges[0].factor; }
        double  bias()   const { return physRanges.isEmpty() ? 0.0 : physRanges[0].bias;   }
        double  minVal() const { return physRanges.isEmpty() ? 0.0 : physRanges[0].minVal; }
        double  maxVal() const { return physRanges.isEmpty() ? 0.0 : physRanges[0].maxVal; }
        QString unit()   const { return physRanges.isEmpty() ? QString() : physRanges[0].unit; }
    };
    QMap<QString, SigEncoding> encodings;
    QStringList encodingDeclarationOrder;

    QString currentEncName;
    int     signalEncDepth  = 0;

    enum Section {
        NONE, NODES, SIGNALS, DIAG_SIGNALS,
        FRAMES, DIAG_FRAMES, SIGNAL_ENC,
        SIGNAL_REPR, SCHEDULE, NODE_ATTR,
        SIGNAL_GROUPS,   // ← AJOUT
        FRAME_GROUPS     // ← AJOUT
    } section = NONE;

    QString currentNodeAttrName;
    bool    inConfigurableFrames = false;
    int     nodeAttrDepth        = 0;

    int scheduleDepth = 0;
    db_SCHEDULE_TABLE currentSchedule;

    while (!f.atEnd())
    {
        QString raw  = f.readLine();
        QString line = raw.trimmed();

        {
            int ci = line.indexOf("//");
            if (ci != -1) line = line.left(ci).trimmed();
        }
        if (line.isEmpty()) continue;

        if (line.startsWith("LIN_description_file")) continue;

        if (line.startsWith("LIN_protocol_version")) {
            QString val = afterEq(line).remove(';').remove('"').trimmed();
            if (!val.isEmpty()) {
                file->linProtocolVersionStr = val;
                file->linProtocolVersion    = val.toDouble();
            }
            continue;
        }
        if (line.startsWith("LIN_language_version")) {
            QString val = afterEq(line).remove(';').remove('"').trimmed();
            if (!val.isEmpty()) {
                file->linLanguageVersionStr = val;
                file->linLanguageVersion    = val.toDouble();
            }
            continue;
        }
        if (line.startsWith("LIN_speed")) {
            QString val = afterEq(line).remove(';').remove('"').trimmed();
            if (!val.isEmpty())
                file->linBaudRate = val.remove("kbps").trimmed().toDouble();
            continue;
        }
        if (line.startsWith("Channel_name")) {
            QString val = afterEq(line).remove(';').remove('"').trimmed();
            file->linChannelName = val;
            continue;
        }

        if (isSectionHeader(line, "Diagnostic_signals"))    { section = DIAG_SIGNALS; continue; }
        if (isSectionHeader(line, "Diagnostic_frames"))     { section = DIAG_FRAMES;  continue; }
        if (isSectionHeader(line, "Signal_encoding_types")) { section = SIGNAL_ENC;   signalEncDepth = 0; currentEncName.clear(); continue; }
        if (isSectionHeader(line, "Signal_representation")) { section = SIGNAL_REPR;  continue; }
        if (isSectionHeader(line, "Schedule_tables"))       { section = SCHEDULE;     scheduleDepth = 0; currentSchedule = db_SCHEDULE_TABLE(); continue; }
        if (isSectionHeader(line, "Node_attributes"))       { section = NODE_ATTR;    nodeAttrDepth = 0; continue; }
        if (isSectionHeader(line, "Nodes"))                 { section = NODES;        continue; }
        if (isSectionHeader(line, "Signals"))               { section = SIGNALS;      continue; }
        if (isSectionHeader(line, "Frames"))                { section = FRAMES;       continue; }
        if (isSectionHeader(line, "Signal_groups")) { section = SIGNAL_GROUPS; continue; }
        if (isSectionHeader(line, "Frame_groups"))   { section = FRAME_GROUPS;  continue; }
        if (line == "}") {
            switch (section) {
            case FRAMES:
            case DIAG_FRAMES:
                currentMsg = nullptr;
                break;
            case SIGNAL_ENC:
                if (signalEncDepth > 0) signalEncDepth--;
                if (signalEncDepth == 0) {
                    currentEncName.clear();
                    section = NONE;
                }
                break;
            case SCHEDULE:
                scheduleDepth--;
                if (scheduleDepth == 1) {
                    if (!currentSchedule.name.isEmpty()) {
                        file->scheduleTables.append(currentSchedule);
                        currentSchedule = db_SCHEDULE_TABLE();
                    }
                } else if (scheduleDepth == 0) {
                    section = NONE;
                }
                break;
            case NODE_ATTR:
                nodeAttrDepth--;
                if (nodeAttrDepth == 1) inConfigurableFrames = false;
                if (nodeAttrDepth == 0) {
                    currentNodeAttrName.clear();
                    section = NONE;
                }
                break;
            case SIGNAL_GROUPS:
            case FRAME_GROUPS:
                section = NONE;   // ← AJOUTER
                break;
            default:
                break;
            }
            continue;
        }

        switch (section)
        {
        // ----------------------------------------------------------------
        case NODES:
        {
            if (line.startsWith("Master:")) {
                QString rest = line.mid(7).remove(';').trimmed();
                QStringList p = rest.split(',');
                if (!p.isEmpty()) {
                    db_NODE node;
                    node.name     = p[0].trimmed();
                    node.isMaster = true;
                    if (p.size() >= 2)
                        node.timeBase = p[1].trimmed().remove("ms").trimmed().toDouble();
                    if (p.size() >= 3)
                        node.jitter = p[2].trimmed().remove("ms").trimmed().toDouble();
                    file->db_nodes.append(node);
                }
            }
            else if (line.startsWith("Slaves:")) {
                QString rest = line.mid(7).remove(';').trimmed();
                for (const QString& s : rest.split(',')) {
                    QString name = s.trimmed();
                    if (!name.isEmpty()) {
                        db_NODE node;
                        node.name     = name;
                        node.isMaster = false;
                        file->db_nodes.append(node);
                    }
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        case SIGNALS:
        {
            QString lineNoSemi = line;
            lineNoSemi.remove(';');

            QRegularExpression re("^(\\w+)\\s*:\\s*(\\d+)\\s*,\\s*(\\{[^}]*\\}|[^,]+)\\s*,\\s*(.+)$");
            auto m = re.match(lineNoSemi.trimmed());
            if (m.hasMatch()) {
                SigDef sd;
                sd.size = m.captured(2).toInt();
                QString initStr = m.captured(3).trimmed();
                sd.initValRaw = initStr;
                if (initStr.startsWith('{')) {
                    QString inner = initStr.mid(1, initStr.indexOf('}') - 1);
                    sd.initVal = inner.split(',').first().trimmed().toDouble();
                } else {
                    sd.initVal = initStr.toDouble();
                }
                QString rest = m.captured(4).trimmed();
                QStringList nodes = rest.split(',', Qt::SkipEmptyParts);
                if (!nodes.isEmpty()) sd.publisher = nodes[0].trimmed();
                for (int i = 1; i < nodes.size(); ++i)
                    sd.subscribers << nodes[i].trimmed();
                sigDefs[m.captured(1)] = sd;
                file->sigDeclarationOrder.append(m.captured(1));
            } else {
                QRegularExpression reNoSub("^(\\w+)\\s*:\\s*(\\d+)\\s*,\\s*(\\{[^}]*\\}|[^,;]+)\\s*,\\s*(\\w+)\\s*$");
                auto m2 = reNoSub.match(lineNoSemi.trimmed());
                if (m2.hasMatch()) {
                    SigDef sd;
                    sd.size = m2.captured(2).toInt();
                    QString initStr = m2.captured(3).trimmed();
                    sd.initValRaw = initStr;
                    if (initStr.startsWith('{')) {
                        QString inner = initStr.mid(1, initStr.indexOf('}') - 1);
                        sd.initVal = inner.split(',').first().trimmed().toDouble();
                    } else {
                        sd.initVal = initStr.toDouble();
                    }
                    sd.publisher = m2.captured(4).trimmed();
                    sigDefs[m2.captured(1)] = sd;
                    file->sigDeclarationOrder.append(m2.captured(1));
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        // FIX 3 : Diagnostic_signals parsés correctement
        case DIAG_SIGNALS:
        {
            QString lineNoSemi = line;
            lineNoSemi.remove(';');

            // Format : "SigName: size, initVal, publisher [, subscriber] ;"
            QRegularExpression re("^(\\w+)\\s*:\\s*(\\d+)\\s*,\\s*(\\{[^}]*\\}|[^,]+)\\s*,\\s*(.+)$");
            auto m = re.match(lineNoSemi.trimmed());
            if (m.hasMatch()) {
                SigDef sd;
                sd.size = m.captured(2).toInt();
                QString initStr = m.captured(3).trimmed();
                sd.initValRaw = initStr;
                if (initStr.startsWith('{')) {
                    QString inner = initStr.mid(1, initStr.indexOf('}') - 1);
                    sd.initVal = inner.split(',').first().trimmed().toDouble();
                } else {
                    sd.initVal = initStr.toDouble();
                }
                QStringList nodes = m.captured(4).split(',', Qt::SkipEmptyParts);
                if (!nodes.isEmpty()) sd.publisher = nodes[0].trimmed();
                for (int i = 1; i < nodes.size(); ++i)
                    sd.subscribers << nodes[i].trimmed();
                // Stocker dans sigDefs pour résolution dans DIAG_FRAMES
                // Ne pas ajouter à sigDeclarationOrder (section séparée)
                sigDefs[m.captured(1)] = sd;
            } else {
                // Sans subscriber
                QRegularExpression reNoSub("^(\\w+)\\s*:\\s*(\\d+)\\s*,\\s*(\\{[^}]*\\}|[^,;]+)\\s*,\\s*(\\w+)\\s*$");
                auto m2 = reNoSub.match(lineNoSemi.trimmed());
                if (m2.hasMatch()) {
                    SigDef sd;
                    sd.size = m2.captured(2).toInt();
                    QString initStr = m2.captured(3).trimmed();
                    sd.initValRaw = initStr;
                    sd.initVal = initStr.startsWith('{')
                                     ? initStr.mid(1, initStr.indexOf('}') - 1).split(',').first().trimmed().toDouble()
                                     : initStr.toDouble();
                    sd.publisher = m2.captured(4).trimmed();
                    sigDefs[m2.captured(1)] = sd;
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        // FIX 2 : Frames — détection Sporadic + Event_triggered
        case FRAMES:
        {
            // Sporadic: "Sporadic: name {"
            QRegularExpression reSporadic("^Sporadic\\s*:\\s*(\\w+)",
                                          QRegularExpression::CaseInsensitiveOption);
            // Event_triggered: "Event_triggered: name, id, schedule, len {"
            QRegularExpression reEvt("^Event_triggered\\s*:\\s*(\\w+)\\s*,\\s*(\\d+)\\s*,\\s*(\\w+)\\s*,\\s*(\\d+)",
                                     QRegularExpression::CaseInsensitiveOption);
            // Unconditional: "FrameName: id, Sender, len {"
            QRegularExpression reUnco("^(\\w+)\\s*:\\s*(\\d+)\\s*,\\s*(\\w+)\\s*,\\s*(\\d+)");

            auto mSpo  = reSporadic.match(line);
            auto mEvt  = reEvt.match(line);
            auto mUnco = reUnco.match(line);

            if (mSpo.hasMatch()) {
                // Sporadic frame — pas d'ID propre, liste de frames inconditionnels
                db_MESSAGE msg;
                msg.name         = mSpo.captured(1);
                msg.ID           = 0;
                msg.len          = 0;
                msg.senderName   = "UNASSIGNED";
                msg.sender       = file->findNodeByIdx(0);
                msg.linFrameType = eLIN_SPORADIC;
                msg.bgColor      = QApplication::palette().color(QPalette::Base);
                msg.fgColor      = QApplication::palette().color(QPalette::WindowText);
                file->messageHandler->addMessage(msg);
                currentMsg = file->messageHandler->findMsgByName(msg.name);
            }
            else if (mEvt.hasMatch()) {
                // Event_triggered frame
                db_MESSAGE msg;
                msg.name                  = mEvt.captured(1);
                msg.ID                    = mEvt.captured(2).toUInt();
                msg.collisionResolveTable = mEvt.captured(3); // schedule table
                msg.len                   = mEvt.captured(4).toUInt();
                msg.senderName            = "UNASSIGNED";
                msg.sender                = file->findNodeByIdx(0);
                msg.linFrameType          = eLIN_EVENT_TRIGGERED;
                msg.bgColor               = QApplication::palette().color(QPalette::Base);
                msg.fgColor               = QApplication::palette().color(QPalette::WindowText);
                file->messageHandler->addMessage(msg);
                currentMsg = file->messageHandler->findMsgByName(msg.name);
            }
            else if (mUnco.hasMatch()) {
                // Unconditional frame
                db_MESSAGE msg;
                msg.name         = mUnco.captured(1);
                msg.ID           = mUnco.captured(2).toUInt();
                msg.len          = mUnco.captured(4).toUInt();
                msg.senderName   = mUnco.captured(3);
                msg.sender       = file->findNodeByName(mUnco.captured(3));
                if (!msg.sender) msg.sender = file->findNodeByIdx(0);
                msg.linFrameType = eLIN_UNCONDITIONAL;
                msg.bgColor      = QApplication::palette().color(QPalette::Base);
                msg.fgColor      = QApplication::palette().color(QPalette::WindowText);
                file->messageHandler->addMessage(msg);
                currentMsg = file->messageHandler->findMsgByID(msg.ID);
            }
            else if (currentMsg) {
                if (currentMsg->linFrameType == eLIN_UNCONDITIONAL) {
                    QRegularExpression reSig("^(\\w+)\\s*,\\s*(\\d+)\\s*;");
                    auto ms = reSig.match(line);
                    if (ms.hasMatch()) {
                        QString sigName = ms.captured(1);
                        DB_SIGNAL sig;
                        sig.name           = sigName;
                        sig.startBit       = ms.captured(2).toInt();
                        sig.signalSize     = sigDefs.contains(sigName) ? sigDefs[sigName].size : 8;
                        sig.intelByteOrder = true;
                        sig.valType        = UNSIGNED_INT;
                        sig.factor         = 1.0;
                        sig.bias           = 0.0;
                        if (sigDefs.contains(sigName)) {
                            sig.initValue    = sigDefs[sigName].initVal;
                            sig.initValueRaw = sigDefs[sigName].initValRaw;
                            if (!sigDefs[sigName].subscribers.isEmpty()) {
                                sig.receiverName = sigDefs[sigName].subscribers[0];
                                sig.receiver     = file->findNodeByName(sig.receiverName);
                            } else {
                                sig.receiverName = "UNASSIGNED";
                                sig.receiver     = file->findNodeByIdx(0);
                            }
                        } else {
                            sig.receiverName = "UNASSIGNED";
                            sig.receiver     = file->findNodeByIdx(0);
                        }
                        sig.parentMessage = currentMsg;
                        currentMsg->sigHandler->addSignal(sig);
                    }
                }
                else if (currentMsg->linFrameType == eLIN_SPORADIC ||
                         currentMsg->linFrameType == eLIN_EVENT_TRIGGERED) {
                    // FIX 2 : "UnconditionalFrame: name ;"
                    if (line.contains("UnconditionalFrame")) {
                        QString fname = line.split(':').last().remove(';').trimmed();
                        if (!fname.isEmpty())
                            currentMsg->associatedFrames.append(fname);
                    }
                    else if (line.contains("CollisionResolveTable")) {
                        currentMsg->collisionResolveTable =
                            line.split(':').last().remove(';').trimmed();
                    }
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        case DIAG_FRAMES:
        {
            QRegularExpression reDF("^(\\w+)\\s*:\\s*(?:0x)?([0-9a-fA-F]+)");
            auto md = reDF.match(line);
            if (md.hasMatch() && line.contains('{')) {
                db_MESSAGE msg;
                msg.name         = md.captured(1);
                msg.ID           = md.captured(2).toUInt(nullptr, 16);
                msg.len          = 8;
                msg.senderName   = "UNASSIGNED";
                msg.sender       = file->findNodeByIdx(0);
                msg.linFrameType = eLIN_DIAGNOSTIC;
                msg.bgColor      = QApplication::palette().color(QPalette::Base);
                msg.fgColor      = QApplication::palette().color(QPalette::WindowText);
                file->messageHandler->addMessage(msg);
                currentMsg = file->messageHandler->findMsgByID(msg.ID);
            }
            else if (currentMsg) {
                QRegularExpression reSig("^(\\w+)\\s*,\\s*(\\d+)\\s*;");
                auto ms = reSig.match(line);
                if (ms.hasMatch()) {
                    QString sigName = ms.captured(1);
                    DB_SIGNAL sig;
                    sig.name           = sigName;
                    sig.startBit       = ms.captured(2).toInt();
                    // FIX 3 : utiliser sigDefs pour les signaux diag parsés
                    sig.signalSize     = sigDefs.contains(sigName) ? sigDefs[sigName].size : 8;
                    sig.intelByteOrder = true;
                    sig.valType        = UNSIGNED_INT;
                    sig.factor         = 1.0;
                    sig.bias           = 0.0;
                    if (sigDefs.contains(sigName)) {
                        sig.initValue    = sigDefs[sigName].initVal;
                        sig.initValueRaw = sigDefs[sigName].initValRaw;
                        if (!sigDefs[sigName].subscribers.isEmpty()) {
                            sig.receiverName = sigDefs[sigName].subscribers[0];
                            sig.receiver     = file->findNodeByName(sig.receiverName);
                        }
                    }
                    if (!sig.receiver) {
                        sig.receiverName = "UNASSIGNED";
                        sig.receiver     = file->findNodeByIdx(0);
                    }
                    sig.parentMessage = currentMsg;
                    currentMsg->sigHandler->addSignal(sig);
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        // FIX 1 : Signal_encoding_types — accumulation de toutes les physical_value ranges
        case SIGNAL_ENC:
        {
            if (line.endsWith('{') && !line.contains(',')) {
                signalEncDepth++;
                if (signalEncDepth == 1) {
                    currentEncName = line;
                    currentEncName.remove('{').trimmed();
                    currentEncName = currentEncName.trimmed();
                    if (!encodings.contains(currentEncName)) {
                        encodings[currentEncName] = SigEncoding();
                        encodingDeclarationOrder.append(currentEncName);
                    }
                }
            }
            else if (!currentEncName.isEmpty()) {
                if (line.startsWith("physical_value")) {
                    // FIX 1 : chaque "physical_value" crée une nouvelle PhysRange
                    encodings[currentEncName].hasPhysical = true;
                    QString rest = line.mid(QString("physical_value").length())
                                       .remove(';').trimmed();
                    if (rest.startsWith(',')) rest = rest.mid(1).trimmed();
                    QStringList p = rest.split(',');
                    if (p.size() >= 4) {
                        DB_SIGNAL::PhysRange range;
                        range.minVal = p[0].trimmed().toDouble();
                        range.maxVal = p[1].trimmed().toDouble();
                        range.factor = p[2].trimmed().toDouble();
                        range.bias   = p[3].trimmed().toDouble();
                        if (p.size() >= 5) {
                            QString unitRaw = p[4].trimmed();
                            if (unitRaw.startsWith('"')) unitRaw = unitRaw.mid(1);
                            if (unitRaw.endsWith('"'))   unitRaw.chop(1);
                            range.unit = unitRaw;
                        }
                        encodings[currentEncName].physRanges.append(range);
                    }
                }
                else if (line.startsWith("logical_value")) {
                    QString rest = line.mid(QString("logical_value").length())
                    .remove(';').trimmed();
                    if (rest.startsWith(',')) rest = rest.mid(1).trimmed();
                    int firstComma = rest.indexOf(',');
                    if (firstComma != -1) {
                        unsigned long rawVal = rest.left(firstComma).trimmed().toULong();
                        QString desc = rest.mid(firstComma + 1).trimmed();
                        desc.remove('"');
                        db_VAL_ENUM_ENTRY entry;
                        entry.value    = rawVal;
                        entry.descript = desc;
                        encodings[currentEncName].valList.append(entry);
                    }
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        // FIX 1 : Signal_representation — propager toutes les ranges au signal
        case SIGNAL_REPR:
        {
            QRegularExpression re("^(\\w+)\\s*:\\s*(.+);");
            auto m = re.match(line);
            if (m.hasMatch()) {
                QString encName = m.captured(1).trimmed();
                if (!file->signalReprOrder.contains(encName))
                    file->signalReprOrder.append(encName);
                if (encodings.contains(encName)) {
                    const SigEncoding& enc = encodings[encName];
                    QStringList sigNames = m.captured(2).split(',', Qt::SkipEmptyParts);
                    for (const QString& sn : sigNames) {
                        QString sigName = sn.trimmed();
                        for (int i = 0; i < file->messageHandler->getCount(); i++) {
                            db_MESSAGE* msg = file->messageHandler->findMsgByIdx(i);
                            DB_SIGNAL* sig  = msg->sigHandler->findSignalByName(sigName);
                            if (sig) {
                                // FIX 1 : copier toutes les ranges
                                sig->physRanges       = enc.physRanges;
                                sig->hasPhysicalValue = enc.hasPhysical;
                                sig->valList          = enc.valList;
                                sig->encodingName     = encName;
                                // Compatibilité : champs simples = première range
                                sig->factor   = enc.factor();
                                sig->bias     = enc.bias();
                                sig->unitName = enc.unit();
                                sig->min      = enc.minVal();
                                sig->max      = enc.maxVal();
                            }
                        }
                    }
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        case SCHEDULE:
        {
            if (line.endsWith('{')) {
                scheduleDepth++;
                if (scheduleDepth == 2) {
                    currentSchedule.name = line;
                    currentSchedule.name.remove('{').trimmed();
                    currentSchedule.name = currentSchedule.name.trimmed();
                    currentSchedule.entries.clear();
                }
            }
            else if (!currentSchedule.name.isEmpty()) {
                QRegularExpression re("^(\\w+)\\s+delay\\s+(\\d+(?:\\.\\d+)?)\\s*ms\\s*;");
                auto m = re.match(line);
                if (m.hasMatch()) {
                    db_SCHEDULE_ENTRY entry;
                    entry.frameName = m.captured(1);
                    entry.delayMs   = m.captured(2).toDouble();
                    currentSchedule.entries.append(entry);
                }
            }
            break;
        }

        // ----------------------------------------------------------------
        case NODE_ATTR:
        {
            if (line.endsWith('{')) {
                nodeAttrDepth++;
                QString keyword = line;
                keyword.remove('{');
                keyword = keyword.trimmed();
                if (nodeAttrDepth == 1) {
                    currentNodeAttrName  = keyword;
                    inConfigurableFrames = false;
                } else if (keyword == "configurable_frames") {
                    inConfigurableFrames = true;
                }
            }
            else if (!currentNodeAttrName.isEmpty()) {
                db_NODE* node = file->findNodeByName(currentNodeAttrName);
                if (!node) break;

                if (inConfigurableFrames) {
                    if (line.contains(';')) {
                        QString frameName = line.remove(';').trimmed();
                        if (!frameName.isEmpty()
                            && !frameName.startsWith("LIN_protocol")
                            && !frameName.startsWith("configured_NAD")
                            && !frameName.startsWith("initial_NAD")
                            && !frameName.startsWith("product_id")
                            && !frameName.startsWith("response_error")
                            && !frameName.startsWith("P2_min")
                            && !frameName.startsWith("ST_min")
                            && !frameName.startsWith("N_As_timeout")
                            && !frameName.startsWith("N_Cr_timeout"))
                        {
                            node->configurableFrames.append(frameName);
                        }
                    }
                }
                else {
                    if (line.startsWith("LIN_protocol")) {
                        QString val = afterEq(line).remove(';').remove('"').trimmed();
                        if (!val.isEmpty()) {
                            node->protocolVersionStr = val;
                            node->protocolVersion    = val.toDouble();
                        }
                    }
                    else if (line.startsWith("configured_NAD")) {
                        QString val = afterEq(line).remove(';').trimmed();
                        if (!val.isEmpty()) node->configuredNAD = val.toInt(nullptr, 0);
                    }
                    else if (line.startsWith("initial_NAD")) {
                        QString val = afterEq(line).remove(';').trimmed();
                        if (!val.isEmpty()) node->initialNAD = val.toInt(nullptr, 0);
                    }
                    else if (line.startsWith("product_id")) {
                        QString rest = afterEq(line).remove(';').trimmed();
                        QStringList p = rest.split(',');
                        if (p.size() >= 3) {
                            node->supplierId = p[0].trimmed().toInt(nullptr, 0);
                            node->functionId = p[1].trimmed().toInt(nullptr, 0);
                            node->variant    = p[2].trimmed().toInt();
                        }
                    }
                    else if (line.startsWith("response_error")) {
                        node->responseErrorSignal = afterEq(line).remove(';').trimmed();
                    }
                    else if (line.startsWith("P2_min")) {
                        QString val = afterEq(line).remove(';').remove("ms").trimmed();
                        if (!val.isEmpty()) node->P2Min = val.toDouble();
                    }
                    else if (line.startsWith("ST_min")) {
                        QString val = afterEq(line).remove(';').remove("ms").trimmed();
                        if (!val.isEmpty()) node->STMin = val.toDouble();
                    }
                    else if (line.startsWith("N_As_timeout")) {
                        QString val = afterEq(line).remove(';').remove("ms").trimmed();
                        if (!val.isEmpty()) node->NASTimeout = val.toDouble();
                    }
                    else if (line.startsWith("N_Cr_timeout")) {
                        QString val = afterEq(line).remove(';').remove("ms").trimmed();
                        if (!val.isEmpty()) node->NCRTimeout = val.toDouble();
                    }
                }
            }
            break;
        }
        // ----------------------------------------------------------------
        case SIGNAL_GROUPS:
        {
            // Format: "GroupName: size { signal1, offset1; ... }"
            // Mais attention : la ligne "GroupName: size {" est déjà passée
            // car on a fait `continue` après `section = SIGNAL_GROUPS`
            // Donc ici on lit le CONTENU du groupe
            QRegularExpression reHdr("^(\\w+)\\s*:\\s*(\\d+)\\s*\\{");
            auto m = reHdr.match(line);
            if (m.hasMatch()) {
                dbFile::db_SIGNAL_GROUP sg;
                sg.name = m.captured(1);
                // Lire les entrées jusqu'à '}'
                while (!f.atEnd()) {
                    QString gline = f.readLine().trimmed();
                    int ci = gline.indexOf("//");
                    if (ci != -1) gline = gline.left(ci).trimmed();
                    if (gline == "}") break;
                    gline.remove(';');
                    QStringList p = gline.split(',', Qt::SkipEmptyParts);
                    if (p.size() >= 2) {
                        sg.signalNames << p[0].trimmed();
                    }
                }
                file->signalGroups.append(sg);
            }
            break;
        }

        // ----------------------------------------------------------------
        case FRAME_GROUPS:
        {
            QRegularExpression reHdr("^(\\w+)\\s+delay\\s+(\\d+(?:\\.\\d+)?)\\s*ms\\s*\\{");
            auto m = reHdr.match(line);
            if (m.hasMatch()) {
                dbFile::db_FRAME_GROUP fg;
                fg.name = m.captured(1);
                while (!f.atEnd()) {
                    QString gline = f.readLine().trimmed();
                    int ci = gline.indexOf("//");
                    if (ci != -1) gline = gline.left(ci).trimmed();
                    if (gline == "}") break;
                    if (gline.endsWith(';')) {
                        gline.chop(1);
                        fg.frames << gline.trimmed();
                    }
                }
                file->frameGroups.append(fg);
            }
            break;
        }
        default:
            break;
        } // switch
    } // while

    f.close();

    // ----------------------------------------------------------------
    // Re-résoudre tous les pointeurs db_NODE*
    // ----------------------------------------------------------------
    for (int i = 0; i < file->messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = file->messageHandler->findMsgByIdx(i);
        if (!msg) continue;
        if (!msg->senderName.isEmpty()) {
            msg->sender = file->findNodeByName(msg->senderName);
            if (!msg->sender) msg->sender = file->findNodeByIdx(0);
        }
        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
            if (!sig) continue;
            if (!sig->receiverName.isEmpty()) {
                sig->receiver = file->findNodeByName(sig->receiverName);
                if (!sig->receiver) sig->receiver = file->findNodeByIdx(0);
            }
        }
    }

    // ----------------------------------------------------------------
    // Signaux orphelins
    // ----------------------------------------------------------------
    QSet<QString> signalsInFrames;
    for (int i = 0; i < file->messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = file->messageHandler->findMsgByIdx(i);
        if (!msg) continue;
        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
            if (sig) signalsInFrames.insert(sig->name);
        }
    }

    for (const QString& sigName : file->sigDeclarationOrder) {
        if (signalsInFrames.contains(sigName)) continue;
        if (!sigDefs.contains(sigName)) continue;
        dbFile::OrphanSig os;
        os.size         = sigDefs[sigName].size;
        os.initValue    = sigDefs[sigName].initVal;
        os.initValueRaw = sigDefs[sigName].initValRaw;
        os.publisher    = sigDefs[sigName].publisher;
        os.subscriber   = sigDefs[sigName].subscribers.isEmpty()
                            ? "" : sigDefs[sigName].subscribers[0];
        os.factor           = 1.0;
        os.bias             = 0.0;
        os.hasPhysicalValue = false;
        file->orphanSignals[sigName] = os;
    }

    // ----------------------------------------------------------------
    // Signal_representation → signaux orphelins (2e passe)
    // ----------------------------------------------------------------
    {
        QFile f2(filename);
        if (f2.open(QIODevice::ReadOnly | QIODevice::Text)) {
            bool inRepr = false;
            while (!f2.atEnd()) {
                QString line2 = f2.readLine().trimmed();
                { int ci = line2.indexOf("//"); if (ci != -1) line2 = line2.left(ci).trimmed(); }
                if (line2.isEmpty()) continue;
                if (line2.startsWith("Signal_representation")) { inRepr = true; continue; }
                if (inRepr && line2 == "}")                    { inRepr = false; continue; }
                if (!inRepr) continue;

                QRegularExpression re("^(\\w+)\\s*:\\s*(.+);");
                auto m = re.match(line2);
                if (!m.hasMatch()) continue;
                QString encName = m.captured(1).trimmed();
                if (!encodings.contains(encName)) continue;
                const SigEncoding& enc = encodings[encName];
                QStringList sigNames = m.captured(2).split(',', Qt::SkipEmptyParts);
                for (const QString& sn : sigNames) {
                    QString sname = sn.trimmed();
                    if (!file->orphanSignals.contains(sname)) continue;
                    dbFile::OrphanSig& os = file->orphanSignals[sname];
                    // FIX 1 : copier toutes les ranges
                    os.physRanges       = enc.physRanges;
                    os.factor           = enc.factor();
                    os.bias             = enc.bias();
                    os.unit             = enc.unit();
                    os.valList          = enc.valList;
                    os.minVal           = enc.minVal();
                    os.maxVal           = enc.maxVal();
                    os.encodingName     = encName;
                    os.hasPhysicalValue = enc.hasPhysical;
                }
            }
            f2.close();
        }
    }

    // ----------------------------------------------------------------
    // Encodages non référencés
    // ----------------------------------------------------------------
    QSet<QString> usedEncNames;
    for (int i = 0; i < file->messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = file->messageHandler->findMsgByIdx(i);
        if (!msg) continue;
        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
            if (sig && !sig->encodingName.isEmpty())
                usedEncNames.insert(sig->encodingName);
        }
    }
    for (auto it = file->orphanSignals.begin(); it != file->orphanSignals.end(); ++it)
        if (!it.value().encodingName.isEmpty())
            usedEncNames.insert(it.value().encodingName);

    for (const QString& encName : encodingDeclarationOrder) {
        if (usedEncNames.contains(encName)) continue;
        if (!encodings.contains(encName)) continue;
        dbFile::RawEncoding re;
        // FIX 1 : copier toutes les ranges
        re.physRanges  = encodings[encName].physRanges;
        re.valList     = encodings[encName].valList;
        re.hasPhysical = encodings[encName].hasPhysical;
        file->unusedEncodings[encName] = re;
        file->unusedEncodingOrder.append(encName);
    }
    file->encodingDeclarationOrder = encodingDeclarationOrder;

    qWarning() << "[LDF] Fin du parsing. Frames:" << file->messageHandler->getCount()
               << "Nodes:" << file->db_nodes.count()
               << "Orphans:" << file->orphanSignals.count();

    return file->messageHandler->getCount() > 0;
}



// ============================================================
// dbSignalHandler
// ============================================================
DB_SIGNAL* dbSignalHandler::findSignalByIdx(int idx)
{
    if (sigs.count() == 0) return nullptr;
    if (idx < 0) return nullptr;
    if (idx >= sigs.count()) return nullptr;
    return &sigs[idx];
}

DB_SIGNAL* dbSignalHandler::findSignalByName(QString name)
{
    if (sigs.count() == 0) return nullptr;
    for (int i = 0; i < sigs.count(); i++)
        if (sigs[i].name.compare(name, Qt::CaseInsensitive) == 0)
            return &sigs[i];
    return nullptr;
}

bool dbSignalHandler::addSignal(DB_SIGNAL &sig)
{
    sigs.append(sig);
    return true;
}

bool dbSignalHandler::removeSignal(DB_SIGNAL *sig)
{
    for (int i = 0; i < getCount(); i++) {
        if (sigs[i].name == sig->name) {
            sigs.removeAt(i);
            break;
        }
    }
    return true;
}

bool dbSignalHandler::removeSignal(int idx)
{
    if (idx < 0 || idx >= sigs.count()) return false;
    sigs.removeAt(idx);
    return true;
}

bool dbSignalHandler::removeSignal(QString name)
{
    bool found = false;
    for (int i = sigs.count() - 1; i >= 0; i--) {
        if (sigs[i].name.compare(name, Qt::CaseInsensitive) == 0) {
            sigs.removeAt(i);
            found = true;
        }
    }
    return found;
}

void dbSignalHandler::removeAllSignals() { sigs.clear(); }
int  dbSignalHandler::getCount()         { return sigs.count(); }
void dbSignalHandler::sort()             { std::sort(sigs.begin(), sigs.end()); }

// ============================================================
// dbMessageHandler
// ============================================================
db_MESSAGE* dbMessageHandler::findMsgByID(uint32_t id)
{
    if (messages.count() == 0) return nullptr;
    for (int i = 0; i < messages.count(); i++)
        if (messages[i].ID == id) return &messages[i];
    return nullptr;
}

db_MESSAGE* dbMessageHandler::findMsgByIdx(int idx)
{
    if (idx < 0 || idx >= messages.count()) return nullptr;
    return &messages[idx];
}

db_MESSAGE* dbMessageHandler::findMsgByName(QString name)
{
    if (messages.count() == 0) return nullptr;
    for (int i = 0; i < messages.count(); i++)
        if (messages[i].name.compare(name, Qt::CaseInsensitive) == 0)
            return &messages[i];
    return nullptr;
}

db_MESSAGE* dbMessageHandler::findMsgByPartialName(QString name)
{
    if (messages.count() == 0) return nullptr;
    for (int i = 0; i < messages.count(); i++)
        if (messages[i].name.contains(name, Qt::CaseInsensitive))
            return &messages[i];
    return nullptr;
}

QList<db_MESSAGE*> dbMessageHandler::findMsgsByNode(db_NODE* node)
{
    QList<db_MESSAGE*> result;
    for (int i = 0; i < messages.count(); i++)
        if (messages[i].sender == node)
            result.append(&messages[i]);
    return result;
}

bool dbMessageHandler::addMessage(db_MESSAGE &msg) { messages.append(msg); return true; }

bool dbMessageHandler::removeMessage(db_MESSAGE *msg)
{
    for (int i = 0; i < messages.count(); i++) {
        if (messages[i].name == msg->name) { messages.removeAt(i); return true; }
    }
    return false;
}

bool dbMessageHandler::removeMessageByIndex(int idx)
{
    if (idx < 0 || idx >= messages.count()) return false;
    messages.removeAt(idx);
    return true;
}

bool dbMessageHandler::removeMessage(uint32_t ID)
{
    for (int i = messages.count() - 1; i >= 0; i--)
        if (messages[i].ID == ID) { messages.removeAt(i); return true; }
    return false;
}

bool dbMessageHandler::removeMessage(QString name)
{
    bool found = false;
    for (int i = messages.count() - 1; i >= 0; i--) {
        if (messages[i].name.compare(name, Qt::CaseInsensitive) == 0) {
            messages.removeAt(i);
            found = true;
        }
    }
    return found;
}

void dbMessageHandler::removeAllMessages() { messages.clear(); }
int  dbMessageHandler::getCount()          { return messages.count(); }

void dbMessageHandler::sort()
{
    std::sort(messages.begin(), messages.end());
    for (int i = 0; i < messages.count(); i++)
        messages[i].sigHandler->sort();
}

bool dbMessageHandler::filterLabeling()                         { return filterLabelingEnabled; }
void dbMessageHandler::setFilterLabeling(bool v)                { filterLabelingEnabled = v; }
MatchingCriteria_t dbMessageHandler::getMatchingCriteria()      { return matchingCriteria; }
void dbMessageHandler::setMatchingCriteria(MatchingCriteria_t c){ matchingCriteria = c; }

// ============================================================
// dbFile
// ============================================================
dbFile::dbFile()
{
    messageHandler = new dbMessageHandler;
    messageHandler->setMatchingCriteria(EXACT_DB);
    messageHandler->setFilterLabeling(false);
    isDirty               = false;
    fileName              = "<Unsaved File>";
    linProtocolVersion    = 2.1;
    linLanguageVersion    = 2.1;
    linProtocolVersionStr = "2.1";
    linLanguageVersionStr = "2.1";
    linBaudRate           = 19.2;
    linChannelName.clear();
    scheduleTables.clear();
    sigDeclarationOrder.clear();
    orphanSignals.clear();
    signalGroups.clear();   // ← ajout
    frameGroups.clear();    // ← ajout
    m_pCluster = nullptr;
}

dbFile::dbFile(const dbFile& cpy) : QObject()
{
    messageHandler = new dbMessageHandler;
    for (int i = 0; i < cpy.messageHandler->getCount(); i++)
        messageHandler->addMessage(*cpy.messageHandler->findMsgByIdx(i));
    messageHandler->setMatchingCriteria(cpy.messageHandler->getMatchingCriteria());
    messageHandler->setFilterLabeling(cpy.messageHandler->filterLabeling());

    fileName              = cpy.fileName;
    filePath              = cpy.filePath;
    assocBuses            = cpy.assocBuses;
    db_nodes              = cpy.db_nodes;
    db_attributes         = cpy.db_attributes;
    isDirty               = cpy.isDirty;
    linProtocolVersion    = cpy.linProtocolVersion;
    linLanguageVersion    = cpy.linLanguageVersion;
    linProtocolVersionStr = cpy.linProtocolVersionStr;
    linLanguageVersionStr = cpy.linLanguageVersionStr;
    linBaudRate           = cpy.linBaudRate;
    linChannelName        = cpy.linChannelName;
    scheduleTables        = cpy.scheduleTables;
    orphanSignals         = cpy.orphanSignals;
    m_pCluster            = cpy.m_pCluster;
    unusedEncodings       = cpy.unusedEncodings;
    unusedEncodingOrder   = cpy.unusedEncodingOrder;
    sigDeclarationOrder   = cpy.sigDeclarationOrder;
    encodingDeclarationOrder = cpy.encodingDeclarationOrder;
    signalReprOrder       = cpy.signalReprOrder;
    signalGroups          = cpy.signalGroups;   // ← ajout
    frameGroups           = cpy.frameGroups;    // ← ajout
}

dbFile& dbFile::operator=(const dbFile& cpy)
{
    if (this != &cpy) {
        delete messageHandler;
        messageHandler = new dbMessageHandler;
        for (int i = 0; i < cpy.messageHandler->getCount(); i++)
            messageHandler->addMessage(*cpy.messageHandler->findMsgByIdx(i));
        messageHandler->setMatchingCriteria(cpy.messageHandler->getMatchingCriteria());
        messageHandler->setFilterLabeling(cpy.messageHandler->filterLabeling());

        fileName              = cpy.fileName;
        filePath              = cpy.filePath;
        assocBuses            = cpy.assocBuses;
        db_nodes              = cpy.db_nodes;
        db_attributes         = cpy.db_attributes;
        isDirty               = cpy.isDirty;
        linProtocolVersion    = cpy.linProtocolVersion;
        linLanguageVersion    = cpy.linLanguageVersion;
        linProtocolVersionStr = cpy.linProtocolVersionStr;
        linLanguageVersionStr = cpy.linLanguageVersionStr;
        linBaudRate           = cpy.linBaudRate;
        linChannelName        = cpy.linChannelName;
        scheduleTables        = cpy.scheduleTables;
        orphanSignals         = cpy.orphanSignals;
        m_pCluster            = cpy.m_pCluster;
        unusedEncodings       = cpy.unusedEncodings;
        unusedEncodingOrder   = cpy.unusedEncodingOrder;
        sigDeclarationOrder   = cpy.sigDeclarationOrder;
        encodingDeclarationOrder = cpy.encodingDeclarationOrder;
        signalReprOrder       = cpy.signalReprOrder;
        signalGroups          = cpy.signalGroups;   // ← ajout
        frameGroups           = cpy.frameGroups;    // ← ajout
    }
    return *this;
}


void dbFile::sort()
{
    std::sort(db_nodes.begin(), db_nodes.end());
    messageHandler->sort();
}

db_NODE* dbFile::findNodeByIdx(int idx)
{
    if (idx < 0 || idx >= db_nodes.count()) return nullptr;
    return &db_nodes[idx];
}

db_NODE* dbFile::findNodeByName(QString name)
{
    for (int i = 0; i < db_nodes.count(); i++)
        if (name.compare(db_nodes[i].name, Qt::CaseInsensitive) == 0)
            return &db_nodes[i];
    return nullptr;
}

db_NODE* dbFile::findNodeByNameAndComment(QString fullname)
{
    for (int i = 0; i < db_nodes.count(); i++) {
        QString nameAndComment = db_nodes[i].comment.isEmpty()
        ? db_nodes[i].name
        : db_nodes[i].name + " - " + db_nodes[i].comment;
        if (fullname.compare(nameAndComment, Qt::CaseInsensitive) == 0)
            return &db_nodes[i];
    }
    return nullptr;
}

QString dbFile::getFullFilename() { return filePath + fileName; }
QString dbFile::getFilename()     { return fileName; }
QString dbFile::getFilenameNoExt(){ return fileName.split(".db")[0]; }
QString dbFile::getPath()         { return filePath; }
int     dbFile::getAssocBus()     { return assocBuses; }
void    dbFile::setAssocBus(int bus) { if (bus >= -1) assocBuses = bus; }

db_ATTRIBUTE* dbFile::findAttributeByName(QString name, db_ATTRIBUTE_TYPE type)
{
    for (int i = 0; i < db_attributes.count(); i++)
        if (db_attributes[i].name.compare(name, Qt::CaseInsensitive) == 0 &&
            (type == ATTR_TYPE_ANY || type == db_attributes[i].attrType))
            return &db_attributes[i];
    return nullptr;
}

db_ATTRIBUTE* dbFile::findAttributeByIdx(int idx)
{
    if (idx < 0 || idx >= db_attributes.count()) return nullptr;
    return &db_attributes[idx];
}

void dbFile::findAttributesByType(db_ATTRIBUTE_TYPE typ, QList<db_ATTRIBUTE>* list)
{
    if (!list) return;
    list->clear();
    foreach (db_ATTRIBUTE attr, db_attributes)
        if (attr.attrType == typ) list->append(attr);
}

void dbFile::setDirtyFlag()   { isDirty = true;  }
void dbFile::clearDirtyFlag() { isDirty = false; }
bool dbFile::getDirtyFlag()   { return isDirty;  }

// ============================================================
// dbFile::saveFile
// ============================================================
bool dbFile::saveFile(QString fileName)
{
    qWarning() << "[LDF] saveFile: début de la sauvegarde vers" << fileName;

    QFile out(fileName);
    if (!out.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qWarning() << "[LDF] ERREUR: impossible d'ouvrir le fichier en écriture:" << fileName;
        return false;
    }
    QTextStream ts(&out);

    // ---- Header ----
    ts << "LIN_description_file;\n";
    ts << "LIN_protocol_version = \"" << linProtocolVersionStr << "\";\n";
    ts << "LIN_language_version = \"" << linLanguageVersionStr << "\";\n";
    ts << "LIN_speed = " << QString::number(linBaudRate, 'f', 1) << " kbps;\n";  // ← 'f',1
    if (!linChannelName.isEmpty())
        ts << "Channel_name = \"" << linChannelName << "\";\n";
    ts << "\n";

    // ---- Nodes ----
    ts << "Nodes {\n";
    QStringList slaves;
    for (const db_NODE& node : db_nodes) {
        if (node.name == "UNASSIGNED") continue;
        if (node.isMaster)
            ts << "  Master: " << node.name << ", "
               << node.timeBase << " ms, " << node.jitter << " ms ;\n";
        else
            slaves << node.name;
    }
    if (!slaves.isEmpty())
        ts << "  Slaves: " << slaves.join(", ") << " ;\n";
    ts << "}\n\n";

    // -------------------------------------------------------
    // Collecte des signaux
    // -------------------------------------------------------
    struct SigInfo {
        int     size             = 8;
        double  initValue        = 0.0;
        QString initValueRaw;
        QString publisher;
        QString subscriber;
        QString encodingName;
        bool    hasPhysicalValue = false;
        QList<DB_SIGNAL::PhysRange> physRanges;
        QList<db_VAL_ENUM_ENTRY>    valList;
    };

    QMap<QString, SigInfo> allSigsMap;
    QSet<QString>          allSigsSet;

    for (int i = 0; i < messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = messageHandler->findMsgByIdx(i);
        if (!msg || msg->linFrameType == eLIN_DIAGNOSTIC) continue;
        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
            if (!sig || allSigsSet.contains(sig->name)) continue;
            allSigsSet.insert(sig->name);
            SigInfo si;
            si.size             = sig->signalSize;
            si.initValue        = sig->initValue;
            si.initValueRaw     = sig->initValueRaw;
            si.publisher        = msg->sender ? msg->sender->name : "UNASSIGNED";
            si.subscriber       = sig->receiverName;
            si.encodingName     = sig->encodingName;
            si.hasPhysicalValue = sig->hasPhysicalValue;
            if (!sig->physRanges.isEmpty()) {
                si.physRanges = sig->physRanges;
            } else if (sig->hasPhysicalValue || sig->factor != 1.0 || sig->bias != 0.0 ||
                       !sig->unitName.isEmpty()) {
                DB_SIGNAL::PhysRange r;
                r.factor = sig->factor;
                r.bias   = sig->bias;
                r.minVal = sig->min;
                r.maxVal = sig->max;
                r.unit   = sig->unitName;
                si.physRanges.append(r);
            }
            si.valList = sig->valList;
            allSigsMap[sig->name] = si;
        }
    }
    for (auto it = orphanSignals.begin(); it != orphanSignals.end(); ++it) {
        const QString&   sigName = it.key();
        if (allSigsSet.contains(sigName)) continue;
        const OrphanSig& os = it.value();
        SigInfo si;
        si.size             = os.size;
        si.initValue        = os.initValue;
        si.initValueRaw     = os.initValueRaw;
        si.publisher        = os.publisher;
        si.subscriber       = os.subscriber;
        si.encodingName     = os.encodingName;
        si.hasPhysicalValue = os.hasPhysicalValue;
        if (!os.physRanges.isEmpty()) {
            si.physRanges = os.physRanges;
        } else if (os.hasPhysicalValue || os.factor != 1.0 || os.bias != 0.0) {
            DB_SIGNAL::PhysRange r;
            r.factor = os.factor; r.bias = os.bias;
            r.minVal = os.minVal; r.maxVal = os.maxVal;
            r.unit   = os.unit;
            si.physRanges.append(r);
        }
        si.valList = os.valList;
        allSigsMap[sigName] = si;
        allSigsSet.insert(sigName);
    }

    QList<QPair<QString, SigInfo>> allSigsList;
    QSet<QString> emitted;
    for (const QString& sigName : sigDeclarationOrder) {
        if (emitted.contains(sigName) || !allSigsMap.contains(sigName)) continue;
        allSigsList.append(qMakePair(sigName, allSigsMap[sigName]));
        emitted.insert(sigName);
    }
    for (auto it = allSigsMap.begin(); it != allSigsMap.end(); ++it) {
        if (!emitted.contains(it.key())) {
            allSigsList.append(qMakePair(it.key(), it.value()));
            emitted.insert(it.key());
        }
    }

    // ---- Signals ----
    ts << "Signals {\n";
    for (const auto& pair : allSigsList) {
        const SigInfo& si = pair.second;
        QString initStr;
        if (!si.initValueRaw.isEmpty())
            initStr = si.initValueRaw;
        else if (si.initValue == qFloor(si.initValue))
            initStr = QString::number((long long)si.initValue);
        else
            initStr = QString::number(si.initValue);

        ts << "  " << pair.first << ": " << si.size << ", " << initStr
           << ", " << si.publisher;
        if (!si.subscriber.isEmpty())
            ts << ", " << si.subscriber;
        ts << " ;\n";
    }
    ts << "}\n\n";

    // ---- Diagnostic_signals ----
    bool hasDiagSignals = false;
    for (int i = 0; i < messageHandler->getCount() && !hasDiagSignals; ++i) {
        db_MESSAGE* msg = messageHandler->findMsgByIdx(i);
        if (msg && msg->linFrameType == eLIN_DIAGNOSTIC && msg->sigHandler->getCount() > 0)
            hasDiagSignals = true;
    }
    if (hasDiagSignals) {
        ts << "Diagnostic_signals {\n";
        for (int i = 0; i < messageHandler->getCount(); ++i) {
            db_MESSAGE* msg = messageHandler->findMsgByIdx(i);
            if (!msg || msg->linFrameType != eLIN_DIAGNOSTIC) continue;
            for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
                DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
                if (sig) {
                    QString initStr = sig->initValueRaw.isEmpty()
                    ? QString::number((long long)sig->initValue)
                    : sig->initValueRaw;
                    ts << "  " << sig->name << ": " << sig->signalSize << ", "
                       << initStr << ", " << sig->receiverName << " ;\n";
                }
            }
        }
        ts << "}\n\n";
    }

    // ---- Frames ----
    ts << "Frames {\n";
    for (int i = 0; i < messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = messageHandler->findMsgByIdx(i);
        if (!msg || msg->linFrameType == eLIN_DIAGNOSTIC) continue;

        switch (msg->linFrameType) {
        case eLIN_UNCONDITIONAL:
            ts << "  " << msg->name << ": " << msg->ID << ", "
               << (msg->sender ? msg->sender->name : "UNASSIGNED")
               << ", " << msg->len << " {\n";
            for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
                DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
                if (sig) ts << "    " << sig->name << ", " << sig->startBit << " ;\n";
            }
            break;
        case eLIN_SPORADIC:
            ts << "  Sporadic: " << msg->name << " {\n";
            for (const QString& fname : msg->associatedFrames)
                ts << "    UnconditionalFrame: " << fname << " ;\n";
            break;
        case eLIN_EVENT_TRIGGERED:
            // ← correction : virgule après le nom, pas deux-points
            ts << "  Event_triggered: " << msg->name
               << ", " << msg->ID
               << ", " << msg->collisionResolveTable
               << ", " << msg->len << " {\n";
            for (const QString& fname : msg->associatedFrames)
                ts << "    UnconditionalFrame: " << fname << " ;\n";
            break;
        default:
            continue;
        }
        ts << "  }\n";
    }
    ts << "}\n\n";

    // ---- Diagnostic_frames ----
    ts << "Diagnostic_frames {\n";
    for (int i = 0; i < messageHandler->getCount(); ++i) {
        db_MESSAGE* msg = messageHandler->findMsgByIdx(i);
        if (!msg || msg->linFrameType != eLIN_DIAGNOSTIC) continue;
        ts << "  " << msg->name << ": 0x"
           << QString::number(msg->ID, 16) << " {\n";
        for (int j = 0; j < msg->sigHandler->getCount(); ++j) {
            DB_SIGNAL* sig = msg->sigHandler->findSignalByIdx(j);
            if (sig) ts << "    " << sig->name << ", " << sig->startBit << " ;\n";
        }
        ts << "  }\n";
    }
    ts << "}\n\n";

    // ---- Node_attributes ----
    ts << "Node_attributes {\n";
    for (const db_NODE& node : db_nodes) {
        if (node.name == "UNASSIGNED" || node.isMaster) continue;
        ts << "  " << node.name << "{\n";
        ts << "    LIN_protocol = \"" << node.protocolVersionStr << "\" ;\n";
        ts << "    configured_NAD = 0x"
           << QString::number(node.configuredNAD, 16).toUpper() << " ;\n";
        ts << "    initial_NAD = 0x"
           << QString::number(node.initialNAD, 16).toUpper() << " ;\n";
        ts << "    product_id = 0x" << QString::number(node.supplierId, 16).toUpper()
           << ", 0x" << QString::number(node.functionId, 16).toUpper()
           << ", " << node.variant << " ;\n";
        if (!node.responseErrorSignal.isEmpty())
            ts << "    response_error = " << node.responseErrorSignal << " ;\n";
        ts << "    P2_min = " << node.P2Min << " ms ;\n";
        ts << "    ST_min = " << node.STMin << " ms ;\n";
        if (node.NASTimeout > 0)
            ts << "    N_As_timeout = " << node.NASTimeout << " ms ;\n";
        if (node.NCRTimeout > 0)
            ts << "    N_Cr_timeout = " << node.NCRTimeout << " ms ;\n";
        if (!node.configurableFrames.isEmpty()) {
            ts << "    configurable_frames {\n";
            for (const QString& fname : node.configurableFrames)
                ts << "      " << fname << " ;\n";
            ts << "    }\n";
        }
        ts << "  }\n";
    }
    ts << "}\n\n";

    // ---- Schedule_tables ----
    if (!scheduleTables.isEmpty()) {
        ts << "Schedule_tables {\n";
        for (const db_SCHEDULE_TABLE& table : scheduleTables) {
            ts << "  " << table.name << " {\n";
            for (const db_SCHEDULE_ENTRY& entry : table.entries)
                ts << "    " << entry.frameName << " delay "
                   << entry.delayMs << " ms ;\n";
            ts << "  }\n";
        }
        ts << "}\n\n";
    }

    // ---- Build encoding groups ----
    QList<QPair<QString, QStringList>> encToSignalsList;
    QMap<QString, int> encIndex;

    for (const QString& encName : encodingDeclarationOrder) {
        bool hasSignals = false;
        for (const auto& pair : allSigsList) {
            QString en = pair.second.encodingName.isEmpty()
            ? pair.first + "_Encoding"
            : pair.second.encodingName;
            if (en == encName) { hasSignals = true; break; }
        }
        if (!hasSignals || encIndex.contains(encName)) continue;
        encIndex[encName] = encToSignalsList.size();
        encToSignalsList.append(qMakePair(encName, QStringList()));
    }

    for (const auto& pair : allSigsList) {
        const SigInfo& info = pair.second;
        bool hasEncoding = (!info.physRanges.isEmpty() || !info.valList.isEmpty()
                            || info.hasPhysicalValue);
        if (!hasEncoding) continue;
        QString encName = info.encodingName.isEmpty()
                              ? pair.first + "_Encoding"
                              : info.encodingName;
        if (!encIndex.contains(encName)) {
            encIndex[encName] = encToSignalsList.size();
            encToSignalsList.append(qMakePair(encName, QStringList()));
        }
        encToSignalsList[encIndex[encName]].second.append(pair.first);
    }
    // ---- Signal_groups ----
    if (!signalGroups.isEmpty()) {
        ts << "Signal_groups {\n";
        for (const db_SIGNAL_GROUP& sg : signalGroups) {
            ts << "  " << sg.name << " {\n";
            for (const QString& sname : sg.signalNames)
                ts << "    " << sname << " ;\n";
            ts << "  }\n";
        }
        ts << "}\n\n";
    }

    // ---- Frame_groups ----
    if (!frameGroups.isEmpty()) {
        ts << "Frame_groups {\n";
        for (const db_FRAME_GROUP& fg : frameGroups) {
            ts << "  " << fg.name << " delay 0 ms {\n";  // delay pas stocké dans votre struct
            for (const QString& fname : fg.frames)
                ts << "    " << fname << " ;\n";
            ts << "  }\n";
        }
        ts << "}\n\n";
    }
    // ---- Signal_encoding_types ----
    ts << "Signal_encoding_types {\n";
    for (const auto& encPair : encToSignalsList) {
        const QString& encName      = encPair.first;
        const QString& firstSigName = encPair.second.first();
        const SigInfo& si           = allSigsMap[firstSigName];

        ts << "  " << encName << " {\n";
        for (const DB_SIGNAL::PhysRange& range : si.physRanges) {
            unsigned long minPhy = (unsigned long)range.minVal;
            unsigned long maxPhy = (unsigned long)range.maxVal;
            ts << "    physical_value, " << minPhy << ", " << maxPhy
               << ", " << range.factor << ", " << range.bias;
            if (!range.unit.isEmpty())
                ts << ", \"" << range.unit << "\"";
            ts << " ;\n";
        }
        for (const db_VAL_ENUM_ENTRY& e : si.valList)
            ts << "    logical_value, " << e.value
               << ", \"" << e.descript << "\" ;\n";
        ts << "  }\n";
    }
    for (const QString& encName : unusedEncodingOrder) {
        if (!unusedEncodings.contains(encName)) continue;
        const RawEncoding& re = unusedEncodings[encName];
        ts << "  " << encName << " {\n";
        for (const DB_SIGNAL::PhysRange& range : re.physRanges) {
            unsigned long minPhy = (unsigned long)range.minVal;
            unsigned long maxPhy = (unsigned long)range.maxVal;
            ts << "    physical_value, " << minPhy << ", " << maxPhy
               << ", " << range.factor << ", " << range.bias;
            if (!range.unit.isEmpty())
                ts << ", \"" << range.unit << "\"";
            ts << " ;\n";
        }
        for (const db_VAL_ENUM_ENTRY& e : re.valList)
            ts << "    logical_value, " << e.value
               << ", \"" << e.descript << "\" ;\n";
        ts << "  }\n";
    }
    ts << "}\n\n";

    // ---- Signal_representation ----
    QMap<QString, QStringList> encSigsMap;
    for (const auto& encPair : encToSignalsList)
        encSigsMap[encPair.first] = encPair.second;

    ts << "Signal_representation {\n";
    for (const QString& encName : signalReprOrder) {
        if (encSigsMap.contains(encName))
            ts << "  " << encName << ": "
               << encSigsMap[encName].join(", ") << " ;\n";
    }
    for (const auto& encPair : encToSignalsList) {
        if (!signalReprOrder.contains(encPair.first))
            ts << "  " << encPair.first << ": "
               << encPair.second.join(", ") << " ;\n";
    }
    ts << "}\n";

    out.close();

    // ← QFileInfo pour décomposition robuste du chemin (Windows + Linux)
    QFileInfo fi(fileName);
    this->fileName = fi.fileName();
    this->filePath = fi.absolutePath() + "/";
    isDirty = false;

    qWarning() << "[LDF] Sauvegarde terminée:" << messageHandler->getCount()
               << "frames, nodes:" << db_nodes.count()
               << "schedule tables:" << scheduleTables.count();
    return true;
}
// ============================================================
// dbHandler
// ============================================================
int dbHandler::createBlankFile()
{
    dbFile newFile;
    db_ATTRIBUTE attr;

    attr.attrType     = ATTR_TYPE_MESSAGE;
    attr.defaultValue = QApplication::palette().color(QPalette::Base).name();
    attr.name         = "GenMsgBackgroundColor";
    attr.valType      = ATTR_STRING;
    newFile.db_attributes.append(attr);

    attr.attrType     = ATTR_TYPE_MESSAGE;
    attr.defaultValue = QApplication::palette().color(QPalette::WindowText).name();
    attr.name         = "GenMsgForegroundColor";
    attr.valType      = ATTR_STRING;
    newFile.db_attributes.append(attr);

    attr.attrType     = ATTR_TYPE_MESSAGE;
    attr.defaultValue = 0;
    attr.name         = "matchingcriteria";
    attr.valType      = ATTR_INT;
    newFile.db_attributes.append(attr);

    attr.attrType     = ATTR_TYPE_MESSAGE;
    attr.defaultValue = 0;
    attr.name         = "filterlabeling";
    attr.valType      = ATTR_INT;
    newFile.db_attributes.append(attr);

    db_NODE falseNode;
    falseNode.name    = "UNASSIGNED";
    falseNode.comment = "Default node if none specified";
    newFile.db_nodes.append(falseNode);
    newFile.setAssocBus(-1);

    loadedFiles.append(newFile);
    return loadedFiles.count();
}

dbFile* dbHandler::loaddbFile(QString filename)
{
    if (filename.endsWith(".ldf", Qt::CaseInsensitive)) {
        dbFile newFile;
        if (parseLDFFile(filename, &newFile)) {
            QStringList parts = filename.split('/');
            newFile.fileName   = parts.last();
            newFile.filePath   = filename.left(
                filename.length() - newFile.fileName.length());
            newFile.assocBuses = 0;
            newFile.isDirty    = false;
            loadedFiles.append(newFile);
            qDebug() << "[LDF] Loaded"
                     << newFile.messageHandler->getCount()
                     << "frames from" << filename;
            return &loadedFiles.last();
        }
        qDebug() << "[LDF] Parse failed:" << filename;
        return nullptr;
    }
    qDebug() << "[dbHandler] Format non supporté:" << filename;
    return nullptr;
}

dbFile* dbHandler::loaddbFile(int idx)
{
    if (idx > -1 && idx < loadedFiles.count()) removedbFile(idx);

    QString filename;
    QFileDialog dialog;
    QSettings settings;

    QStringList filters;
    filters.append(QString(tr("LDF File (*.ldf)")));

    dialog.setDirectory(
        settings.value("ldf/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);

    if (dialog.exec() == QDialog::Accepted) {
        filename = dialog.selectedFiles()[0];
        settings.setValue("ldf/LoadSaveDirectory", dialog.directory().path());
        return loaddbFile(filename);
    }
    return nullptr;
}

void dbHandler::savedbFile(int idx)
{
    QSettings settings;
    if (idx < 0 || idx >= loadedFiles.count()) return;

    QString filename;
    QFileDialog dialog;

    QStringList filters;
    filters.append(QString(tr("LDF File (*.ldf)")));

    dialog.setDirectory(
        settings.value("ldf/LoadSaveDirectory", dialog.directory().path()).toString());
    dialog.setFileMode(QFileDialog::AnyFile);
    dialog.setNameFilters(filters);
    dialog.setViewMode(QFileDialog::Detail);
    dialog.setAcceptMode(QFileDialog::AcceptSave);
    dialog.selectFile(loadedFiles[idx].getFullFilename());

    if (dialog.exec() == QDialog::Accepted) {
        filename = dialog.selectedFiles()[0];
        if (!filename.contains('.')) filename += ".ldf";
        loadedFiles[idx].saveFile(filename);
        settings.setValue("ldf/LoadSaveDirectory", dialog.directory().path());
    }
}

void dbHandler::removedbFile(int idx)
{
    if (idx < 0 || idx >= loadedFiles.count()) return;
    loadedFiles.removeAt(idx);
}

void dbHandler::removeAllFiles() { loadedFiles.clear(); }

void dbHandler::swapFiles(int pos1, int pos2)
{
    if (pos1 < 0 || pos1 >= loadedFiles.count() ||
        pos2 < 0 || pos2 >= loadedFiles.count()) return;
    loadedFiles.swapItemsAt(pos1, pos2);
}

db_MESSAGE* dbHandler::findMessage(const LINFrame &frame)
{
    for (int i = 0; i < loadedFiles.count(); i++) {
        if (loadedFiles[i].getAssocBus() == 0 ||
            frame.bus == loadedFiles[i].getAssocBus()) {
            db_MESSAGE* msg =
                loadedFiles[i].messageHandler->findMsgByID(frame.frameId());
            if (msg) return msg;
        }
    }
    return nullptr;
}

db_MESSAGE* dbHandler::findMessage(uint32_t id)
{
    for (int i = 0; i < loadedFiles.count(); i++) {
        db_MESSAGE* msg = loadedFiles[i].messageHandler->findMsgByID(id);
        if (msg) return msg;
    }
    return nullptr;
}

db_MESSAGE* dbHandler::findMessageForFilter(uint32_t id,
                                            MatchingCriteria_t* matchingCriteria)
{
    for (int i = 0; i < loadedFiles.count(); i++) {
        if (loadedFiles[i].messageHandler->filterLabeling()) {
            db_MESSAGE* msg = loadedFiles[i].messageHandler->findMsgByID(id);
            if (msg) {
                if (matchingCriteria)
                    *matchingCriteria =
                        loadedFiles[i].messageHandler->getMatchingCriteria();
                return msg;
            }
        }
    }
    return nullptr;
}

db_MESSAGE* dbHandler::findMessage(const QString msgName)
{
    for (int i = 0; i < loadedFiles.count(); i++) {
        db_MESSAGE* msg = loadedFiles[i].messageHandler->findMsgByName(msgName);
        if (msg) return msg;
    }
    return nullptr;
}

db_MESSAGE* dbHandler::findMessage(const QString msgName,
                                   const QString nodeName,
                                   const QString fileNameNoExt)
{
    for (int i = 0; i < loadedFiles.count(); i++) {
        dbFile* file = getFileByIdx(i);
        if (file->getFilenameNoExt() == fileNameNoExt) {
            for (int f = 0; f < file->messageHandler->getCount(); f++) {
                db_MESSAGE* msg = file->messageHandler->findMsgByIdx(f);
                if (msg && msg->name == msgName && msg->sender->name == nodeName)
                    return msg;
            }
        }
    }
    return nullptr;
}

db_MESSAGE* dbHandler::findMessage(const QString msgName,
                                   const QString fullyQualifiedNodeName)
{
    QStringList parts = fullyQualifiedNodeName.split(Utility::fullyQualifiedNameSeperator);
    if (parts.count() != 2) return nullptr;
    return findMessage(msgName, parts[1], parts[0]);
}

int     dbHandler::getFileCount()        { return loadedFiles.count(); }

dbFile* dbHandler::getFileByIdx(int idx)
{
    if (idx < 0 || idx >= loadedFiles.count()) return nullptr;
    return &loadedFiles[idx];
}

dbFile* dbHandler::getFileByName(QString name)
{
    for (int i = 0; i < loadedFiles.count(); i++)
        if (loadedFiles[i].getFilename().compare(name, Qt::CaseInsensitive) == 0)
            return &loadedFiles[i];
    return nullptr;
}

dbHandler::dbHandler()
{
    QSettings settings;
    int filecount = settings.value("ldf/FileCount", 0).toInt();
    for (int i = 0; i < filecount; i++) {
        QString filename =
            settings.value("ldf/Filename_" + QString::number(i), "").toString();
        dbFile* file = loaddbFile(filename);
        if (file) {
            int bus = settings.value("ldf/AssocBus_" + QString::number(i), 0).toInt();
            file->setAssocBus(bus);
        }
    }
}

dbHandler* dbHandler::getReference()
{
    if (!instance) instance = new dbHandler();
    return instance;
}
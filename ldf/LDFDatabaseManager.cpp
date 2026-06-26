#include "LDFDatabaseManager.h"

#include "qsettings.h"

LDFDatabaseManager* LDFDatabaseManager::m_pouLdfDatabaseManager = nullptr;

LDFDatabaseManager::LDFDatabaseManager(void) {
  m_pouLDFClsuter = nullptr;
  m_pouLdfDatabaseManager = nullptr;
  m_strCurrentLDFFilePath = "";
  m_bDisplayInHex = 0;
  readLDFSettings();
  m_bDocumentModified = false;
  m_pfCreateLDFCluster = nullptr;
}

bool LDFDatabaseManager::bIsDocumentModified() { return m_bDocumentModified; }

void LDFDatabaseManager::setDocumentModified(bool bModified) {
  m_bDocumentModified = bModified;
  emit DocumentModified();
}

void LDFDatabaseManager::DeleteDatabaseManager() {
  if (nullptr != m_pouLdfDatabaseManager) {
    delete m_pouLdfDatabaseManager;
    m_pouLdfDatabaseManager = nullptr;
  }
}

bool LDFDatabaseManager::bIsDisplayHexOn() { return m_bDisplayInHex; }

void LDFDatabaseManager::SetDisplayHex(bool bMode) {
  m_bDisplayInHex = bMode;
  emit DisplayModeChanged();
}

LDFDatabaseManager::~LDFDatabaseManager(void) { saveLDFSettings(); }

void LDFDatabaseManager::saveLDFSettings() {
  QSettings settings("RBEI-ETAS", "BUSMASTER_LDFEditor");

  settings.beginGroup("LDFUISettings");
  settings.setValue("hexMode", m_bDisplayInHex);

  settings.setValue("recentFileList", m_strRecentFileList);

  settings.endGroup();
}

void LDFDatabaseManager::readLDFSettings() {
  QSettings settings("RBEI-ETAS", "BUSMASTER_LDFEditor");

  settings.beginGroup("LDFUISettings");

  m_bDisplayInHex = settings.value("hexMode").value<bool>();

  m_strRecentFileList.clear();
  m_strRecentFileList = settings.value("recentFileList").toStringList();

  settings.endGroup();
}

LDFDatabaseManager* LDFDatabaseManager::GetDatabaseManager() {
  if (nullptr == m_pouLdfDatabaseManager) {
    m_pouLdfDatabaseManager = new LDFDatabaseManager();
  }
  return m_pouLdfDatabaseManager;
}

ICluster* LDFDatabaseManager::GetLDFCluster() {
    qWarning() << "GetLDFCluster: entry";
    if (nullptr == m_pfCreateLDFCluster) {
        qWarning() << "GetLDFCluster: m_pfCreateLDFCluster is null, trying to load DBManager.dll";
        mDbManagerDll.setFileName("DBManager");
        qWarning() << "GetLDFCluster: DLL file name set to" << mDbManagerDll.fileName();

        if (!mDbManagerDll.load()) {
            qWarning() << "GetLDFCluster: Failed to load DBManager.dll, error:" << mDbManagerDll.errorString();
            return nullptr;
        }
        qWarning() << "GetLDFCluster: DBManager.dll loaded successfully";

        m_pfCreateLDFCluster = (CreateLDFCluster)mDbManagerDll.resolve("CreateLDFCluster");
        if (nullptr == m_pfCreateLDFCluster) {
            qWarning() << "GetLDFCluster: Failed to resolve CreateLDFCluster";
            return nullptr;
        }
        qWarning() << "GetLDFCluster: CreateLDFCluster resolved";
    }

    if (nullptr == m_pouLDFClsuter) {
        qWarning() << "GetLDFCluster: Creating cluster via CreateLDFCluster";
        m_pfCreateLDFCluster(&m_pouLDFClsuter);
        if (nullptr == m_pouLDFClsuter) {
            qWarning() << "GetLDFCluster: CreateLDFCluster returned null cluster";
            return nullptr;
        }
        qWarning() << "GetLDFCluster: Cluster created";
    }
    return m_pouLDFClsuter;
}

int LDFDatabaseManager::SetLDFFilePath(const std::string& strFilePath) {
  m_strCurrentLDFFilePath = strFilePath;

  if (strFilePath != "") {
    auto itr = m_strRecentFileList.removeAll(strFilePath.c_str());
    m_strRecentFileList.prepend(strFilePath.c_str());
  }

  emit DataUpdated();
  return 0;
}

int LDFDatabaseManager::GetMaxRecentFileCount() { return 5; }

void LDFDatabaseManager::GetRecentFileList(QStringList& strRecentFileList) {
  strRecentFileList = m_strRecentFileList;
}

int LDFDatabaseManager::LoadLDFFilePath(std::string& strFilePath) { return 0; }

std::string LDFDatabaseManager::GetLDFFilePath() {
  return m_strCurrentLDFFilePath;
}
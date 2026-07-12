#include "Window.h"
#include <shellapi.h>

DropTarget::DropTarget(Window *pWindow) : m_refCount(1), m_pWindow(pWindow) {}

HRESULT STDMETHODCALLTYPE DropTarget::QueryInterface(REFIID riid,
                                                     void **ppvObject) {
  if (riid == IID_IUnknown || riid == IID_IDropTarget) {
    *ppvObject = this;
    AddRef();
    return S_OK;
  }
  *ppvObject = nullptr;
  return E_NOINTERFACE;
}

ULONG STDMETHODCALLTYPE DropTarget::AddRef() {
  return InterlockedIncrement(&m_refCount);
}

ULONG STDMETHODCALLTYPE DropTarget::Release() {
  LONG count = InterlockedDecrement(&m_refCount);
  if (count == 0) {
    delete this;
  }
  return count;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragEnter(IDataObject *pDataObj,
                                                DWORD grfKeyState, POINTL pt,
                                                DWORD *pdwEffect) {
  m_pWindow->SetHovered(true);
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragOver(DWORD grfKeyState, POINTL pt,
                                               DWORD *pdwEffect) {
  m_pWindow->SetHovered(true);
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::DragLeave() {
  m_pWindow->SetHovered(false);
  return S_OK;
}

HRESULT STDMETHODCALLTYPE DropTarget::Drop(IDataObject *pDataObj,
                                           DWORD grfKeyState, POINTL pt,
                                           DWORD *pdwEffect) {
  m_pWindow->SetHovered(false);

  FORMATETC fmt = {CF_HDROP, nullptr, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
  STGMEDIUM stg = {0};
  if (SUCCEEDED(pDataObj->GetData(&fmt, &stg))) {
    HDROP hDrop = (HDROP)stg.hGlobal;
    UINT count = DragQueryFileW(hDrop, 0xFFFFFFFF, nullptr, 0);
    std::vector<std::wstring> files;
    for (UINT i = 0; i < count; ++i) {
      UINT length = DragQueryFileW(hDrop, i, nullptr, 0);
      std::wstring path(length, L'\0');
      DragQueryFileW(hDrop, i, &path[0], length + 1);
      files.push_back(path);
    }
    m_pWindow->NotifyFilesDropped(files);
    ReleaseStgMedium(&stg);
  }
  *pdwEffect = DROPEFFECT_COPY;
  return S_OK;
}

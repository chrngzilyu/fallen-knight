#pragma once
#include "Common.h"

class SoundManager {
private:
    int rot = 0;

    bool fileExists(const TCHAR* filepath) {
        DWORD dwAttrib = GetFileAttributes(filepath);
        return (dwAttrib != INVALID_FILE_ATTRIBUTES && !(dwAttrib & FILE_ATTRIBUTE_DIRECTORY));
    }

    void preload(const TCHAR* filepath, const TCHAR* aliasBase) {
        if (!fileExists(filepath)) {
            _stprintf_s(g_audioErrorMsg, 256, _T("【路径错误】找不到音效: %s"), filepath);
            return;
        }
        TCHAR cmd[256];
        for (int i = 0; i < 5; i++) {
            _stprintf_s(cmd, 256, _T("open \"%s\" alias %s_%d"), filepath, aliasBase, i);
            mciSendString(cmd, NULL, 0, NULL);
        }
    }

public:
    void loadAllSfx() {
        preload(_T("sounds\\ui_confirm.wav"), _T("ui_confirm"));
        preload(_T("sounds\\ui_cancel.wav"), _T("ui_cancel"));
        preload(_T("sounds\\light_attack.wav"), _T("light_attack"));
        preload(_T("sounds\\heavy_attack.wav"), _T("heavy_attack"));
        preload(_T("sounds\\dash.wav"), _T("dash"));
        preload(_T("sounds\\block.wav"), _T("block"));
        preload(_T("sounds\\hurt.wav"), _T("hurt"));
        preload(_T("sounds\\hiss.wav"), _T("hiss"));
    }

    void playBGM(const TCHAR* filepath) {
        if (!fileExists(filepath)) {
            _stprintf_s(g_audioErrorMsg, 256, _T("【路径错误】找不到BGM: %s (请检查目录)"), filepath);
            return;
        }
        _tcscpy_s(g_audioErrorMsg, 256, _T(""));

        TCHAR cmd[256];
        mciSendString(_T("close bgm"), NULL, 0, NULL);
        _stprintf_s(cmd, 256, _T("open \"%s\" alias bgm"), filepath);
        mciSendString(cmd, NULL, 0, NULL);
        mciSendString(_T("setaudio bgm volume to 30"), NULL, 0, NULL);

        mciSendString(_T("play bgm repeat"), NULL, 0, NULL);
    }

    void stopBGM() {
        mciSendString(_T("stop bgm"), NULL, 0, NULL);
        mciSendString(_T("close bgm"), NULL, 0, NULL);
    }

    void playSfx(const TCHAR* aliasBase) {
        TCHAR cmd[256];
        rot = (rot + 1) % 5;
        _stprintf_s(cmd, 256, _T("play %s_%d from 0"), aliasBase, rot);
        mciSendString(cmd, NULL, 0, NULL);
    }
};

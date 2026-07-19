## HOTFIX: TagManager Warning C4996 の解消
- **原因**: `TagLib::AudioProperties::length()` がTagLib 2.0以降で非推奨（deprecated）となっており、コンパイル時にWarning C4996が発生していた。
- **対応内容**: `src/TagManager.cpp` の該当箇所を推奨される `lengthInSeconds()` メソッドの呼び出しへと置換し、警告を解消した。

$lines = Get-Content src\Widgets.cpp
$lines[311] = '        } else {'
$lines[312] = '            if (ctx.nextArtBitmap) {'
$lines | Set-Content src\Widgets.cpp -Encoding UTF8

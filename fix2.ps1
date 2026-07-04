$content = Get-Content src\Renderer.cpp -Raw
$content = $content.TrimEnd()
if ($content.EndsWith("}")) {
    $content = $content.Substring(0, $content.LastIndexOf("}"))
}
$content = $content.TrimEnd()
if ($content.EndsWith("}")) {
    $content = $content.Substring(0, $content.LastIndexOf("}"))
}
$content = $content.TrimEnd()
Set-Content src\Renderer.cpp $content -Encoding UTF8

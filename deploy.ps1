$target = "C:\Games\Skyrim_Test\Mod Organizer\mods\EasyOutfit"

$srcDll = "$PSScriptRoot\build\windows\x64\release\EasyOutfit.dll"

if (-not (Test-Path -LiteralPath $srcDll)) {
    Write-Error "EasyOutfit.dll not found at '$srcDll'. Please build first: xmake -y"
    exit 1
}

$destPlugins = "$target\SKSE\Plugins"
New-Item -ItemType Directory -Force -Path $destPlugins | Out-Null

# 1. Deploy DLL
try {
    Copy-Item -LiteralPath $srcDll -Destination "$destPlugins\EasyOutfit.dll" -Force -ErrorAction Stop
} catch {
    Write-Warning "DLL is likely locked by a running process. Attempting fallback rename..."
    Move-Item -LiteralPath "$destPlugins\EasyOutfit.dll" -Destination "$destPlugins\EasyOutfit.dll.old" -Force -ErrorAction SilentlyContinue
    Copy-Item -LiteralPath $srcDll -Destination "$destPlugins\EasyOutfit.dll" -Force
}

# 2. Deploy default EasyOutfit.json if missing
$defaultConfig = @'
{
  "enabled": true,
  "suppressBeastForm": true,
  "toggleKey": 24,
  "activeOutfit": "",
  "language": "auto"
}
'@

$cfgPath = "$destPlugins\EasyOutfit.json"
if (-not (Test-Path -LiteralPath $cfgPath)) {
    [System.IO.File]::WriteAllText($cfgPath, $defaultConfig, [System.Text.Encoding]::UTF8)
    Write-Host "[CREATED] $cfgPath" -ForegroundColor Cyan
}

# 3. Deploy default EasyOutfit_Presets.json if missing
$defaultPresets = @'
{
  "outfits": []
}
'@

$presetsPath = "$destPlugins\EasyOutfit_Presets.json"
if (-not (Test-Path -LiteralPath $presetsPath)) {
    [System.IO.File]::WriteAllText($presetsPath, $defaultPresets, [System.Text.Encoding]::UTF8)
    Write-Host "[CREATED] $presetsPath" -ForegroundColor Cyan
}

# 4. Deploy translation JSON files
Get-ChildItem -Path "$PSScriptRoot\SKSE\Plugins\EasyOutfitStrings*.json" | ForEach-Object {
    Copy-Item -LiteralPath $_.FullName -Destination "$destPlugins\$($_.Name)" -Force
    Write-Host "[DEPLOYED] $($_.Name)" -ForegroundColor Cyan
}

Write-Host "[SUCCESS] Deployed EasyOutfit to: $target" -ForegroundColor Green
$dllItem = Get-Item "$destPlugins\EasyOutfit.dll"
Write-Host "DLL Size: $($dllItem.Length) bytes, LastWriteTime: $($dllItem.LastWriteTime)" -ForegroundColor Green

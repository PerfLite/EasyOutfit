# Package EasyOutfit for Nexus Mods release
$version = "1.0.0"
$distDir = "$PSScriptRoot\dist"
$zipPath = "$distDir\EasyOutfit-v$version.zip"
$stageDir = "$distDir\staging"

$srcDll = "$PSScriptRoot\build\windows\x64\release\EasyOutfit.dll"
if (-not (Test-Path -LiteralPath $srcDll)) {
    Write-Error "EasyOutfit.dll not found at '$srcDll'. Please build first with 'xmake -y'."
    exit 1
}

# Clean staging & dist
if (Test-Path -LiteralPath $stageDir) { Remove-Item -LiteralPath $stageDir -Recurse -Force }
if (-not (Test-Path -LiteralPath $distDir)) { New-Item -ItemType Directory -Path $distDir | Out-Null }
if (Test-Path -LiteralPath $zipPath) { Remove-Item -LiteralPath $zipPath -Force }

# Create folder structure
$targetPlugins = "$stageDir\SKSE\Plugins"
New-Item -ItemType Directory -Force -Path $targetPlugins | Out-Null

# Copy DLL
Copy-Item -LiteralPath $srcDll -Destination "$targetPlugins\EasyOutfit.dll" -Force

# Create default clean EasyOutfit.json
$defaultConfig = @'
{
  "enabled": true,
  "suppressBeastForm": true,
  "toggleKey": 24,
  "toggleKeyName": "O",
  "language": "auto",
  "selectedOutfit": ""
}
'@
[System.IO.File]::WriteAllText("$targetPlugins\EasyOutfit.json", $defaultConfig, [System.Text.Encoding]::UTF8)

# Create zip archive
Compress-Archive -Path "$stageDir\*" -DestinationPath $zipPath -Force
Remove-Item -LiteralPath $stageDir -Recurse -Force

Write-Host "[SUCCESS] Release package created at:" -ForegroundColor Green
Write-Host "  $zipPath" -ForegroundColor Cyan

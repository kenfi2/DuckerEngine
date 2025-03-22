$SaveFolder = $args[0]
if ($args.count -lt 1) {
    Write-Host "Missing save folder parameter"
    exit 1
}

if (Test-Path $SaveFolder\*) {
    Write-Host "DXC is already installed: $SaveFolder"
    exit 0
}

New-Item -ItemType Directory -Force -Path $SaveFolder

$JSONURL = "https://api.github.com/repos/microsoft/DirectXShaderCompiler/releases/latest"
$JSON = Invoke-WebRequest -UseBasicParsing -Uri $JSONURL
$ParsedJSON = ConvertFrom-Json -InputObject $JSON.Content
$assets = $ParsedJSON.assets

foreach ($Asset in $assets) {
    if ($Asset.name -match "dxc.*.zip") {
        $DownloadURL = $Asset.browser_download_url
        $ZIPPath = Join-Path -Path $SaveFolder -ChildPath "dxc.zip"
        Invoke-WebRequest -UseBasicParsing -Uri $DownloadURL -OutFile $ZIPPath
        Expand-Archive -Path $ZIPPath -DestinationPath $SaveFolder -Force
        Remove-Item -Path $ZIPPath
        Write-Host "Successfully downloaded DXC (${Asset.name})"
        exit 0
    }
}

Write-Host "Something went wrong, couldn't download DXC"
exit 1
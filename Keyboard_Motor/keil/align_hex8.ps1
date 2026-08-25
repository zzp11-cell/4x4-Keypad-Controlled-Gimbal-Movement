# align_hex8.ps1 - 把 Intel HEX 最后一条数据记录补齐到 8 字节
# 用途: MSPM0 BSL 烧录要求起始地址和数据长度都 8 字节对齐,
#       Keil 默认输出的 hex 末尾可能是 4 字节,BSL 会报
#       "Start address or data length ... not 8 byte aligned"。
#       在 Keil 的 After Build 调用本脚本,自动把末尾补 0xFF 到 8 字节边界。
# 用法: powershell -ExecutionPolicy Bypass -File align_hex8.ps1 "<hex路径>"

param([Parameter(Mandatory = $true)][string]$HexPath)

if (-not (Test-Path -LiteralPath $HexPath)) {
    Write-Error "align_hex8: file not found: $HexPath"
    exit 1
}

$lines = Get-Content -LiteralPath $HexPath -Encoding ASCII

# 找最后一条数据记录 (record type = 00)
$lastDataIdx = -1
for ($i = $lines.Count - 1; $i -ge 0; $i--) {
    if ($lines[$i] -match '^:[0-9A-Fa-f]{2}[0-9A-Fa-f]{4}00') {
        $lastDataIdx = $i
        break
    }
}
if ($lastDataIdx -lt 0) {
    Write-Host "align_hex8: no data record found, skip"
    exit 0
}

$line       = $lines[$lastDataIdx]
$byteCount  = [Convert]::ToInt32($line.Substring(1, 2), 16)

if (($byteCount % 8) -eq 0) {
    Write-Host "align_hex8: already 8-byte aligned ($byteCount B), skip"
    exit 0
}

$padCount     = 8 - ($byteCount % 8)
$newByteCount = $byteCount + $padCount
$addr         = $line.Substring(3, 4)
$data         = $line.Substring(9, $byteCount * 2)
$newData      = $data + ('FF' * $padCount)

# 重组记录并重算 checksum (0x100 - (sum & 0xFF)) & 0xFF
$hex = '{0:X2}{1}00{2}' -f $newByteCount, $addr, $newData
$sum = 0
for ($j = 0; $j -lt $hex.Length; $j += 2) {
    $sum += [Convert]::ToInt32($hex.Substring($j, 2), 16)
}
$chk = (0x100 - ($sum -band 0xFF)) -band 0xFF
$lines[$lastDataIdx] = ':' + $hex + ('{0:X2}' -f $chk)

Set-Content -LiteralPath $HexPath -Value $lines -Encoding ASCII
Write-Host ("align_hex8: padded {0} -> {1} bytes @0x{2}" -f $byteCount, $newByteCount, $addr)

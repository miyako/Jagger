//%attributes = {"invisible":true}
var $Jagger : cs:C1710.Jagger
$Jagger:=cs:C1710.Jagger.new()
$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
$status:=$Jagger.split($text)
ALERT:C41($status.data.join(","))
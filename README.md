# Jagger
形態素解析器

## つかいかた

```4d
var $Jagger : cs.Jagger.Jagger
$Jagger:=cs.Jagger.Jagger.new()
$status:=$Jagger.words("Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。")
$pos:=$status.extract("pos")
ALERT($pos.join("\r"))
```

## aknowlegements

* http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/jagger/index.ja.html

# Jagger
形態素解析器

 ## 辞書の管理

辞書は別途GitHubからダウンロードする設計です。  
まずZip形式で圧縮しておきます。  

* ファイル名は常に`model.zip`
  
```4d
$model:=Folder("/RESOURCES/model/kyoto+kwdlc")
$zip:=cs._Zip.new($model)
$zip.create()
```

リリース名を辞書名に合わせます。

https://github.com/miyako/Jagger/releases/tag/kyoto%2Bkwdlc

Releasesから.zip形式の辞書をダウンロードして展開します。

```4d
$model:=cs._Model.new("kyoto+kwdlc")
If (Not($model.exists))
	$model.install()
End if
```

## つかいかた

```4d
var $Jagger : cs.Jagger
$Jagger:=cs.Jagger.new()

$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
$status:=$Jagger.split($text)
ALERT($status.join(","))
```

<img src="https://github.com/user-attachments/assets/f7932cc1-8250-4aa0-b436-8418290fd094" width=400 height=auto />

```4d
var $Jagger : cs.Jagger
$Jagger:=cs.Jagger.new()
$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
$status:=$Jagger.analyze($text)
ALERT(JSON Stringify($status; *))
```

<img src="https://github.com/user-attachments/assets/f9c73247-5a62-4fb1-ae26-94a4d9091736" width=400 height=auto />

## 注記

* 改行コード（両プラットフォーム`\n`）までが解析されます。

コンストラクターが必要に応じてGitHubからデフォルトの辞書をインストールします。

## aknowlegements

* http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/jagger/index.ja.html

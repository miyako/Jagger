# Jagger
形態素解析器

実行ファイル`jagger`は`stdIn` `stdOut`ではなく，コマンドラインと対話するバージョンです。

<img src="https://github.com/user-attachments/assets/2426962a-b4ce-48a2-a029-78d4ea909db3" width=500 height=auto />

デフォルトのモデルファイルは`../model/kyoto+kwdlc/pattern`です。



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
$model:=cs.Jagger.Model.new("kyoto+kwdlc")
If (Not($model.exists))
	$model.install()
End if
```

## つかいかた（同期モード）

```4d
var $Jagger : cs.Jagger.Jagger
$Jagger:=cs.Jagger.Jagger.new()

$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
$status:=$Jagger.split($text)
ALERT($status.join(","))
```

<img src="https://github.com/user-attachments/assets/f7932cc1-8250-4aa0-b436-8418290fd094" width=400 height=auto />

```4d
var $Jagger : cs.Jagger.Jagger
$Jagger:=cs.Jagger.Jagger.new()
$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
$status:=$Jagger.tokenize($text)
ALERT(JSON Stringify($status; *))
```

<img src="https://github.com/user-attachments/assets/f9c73247-5a62-4fb1-ae26-94a4d9091736" width=400 height=auto />

## つかいかた（非同期モード）

```4d
#DECLARE($params : Object)

If (Count parameters=0)
	
	CALL WORKER("Jagger"; Current method name; {})
	
Else 
	
	var Jagger : cs.Jagger.Jagger
	If (Jagger=Null)
		Jagger:=cs.Jagger.Jagger.new()
	End if 
	
	$text:="Jagger は、辞書に基づく単語分割のための最長一致法と、機械学習の分類器の事前計算に着想を得た、特徴パターンに基づく高速・高精度・省メモリの形態素解析器です。"
	Jagger.split($text; True)
	
	//Jagger.quit()
	
End if
```
<img src="https://github.com/user-attachments/assets/189b39b6-21c9-413a-8462-8b51c559b9b6" width=800 height=auto />



実行ファイルを起動したまま，CGIのように標準入出力で対話します。毎回，辞書やモデルをロードしないので効率的です。`.quit()`するまで`onData`コールバック関数が呼ばれます。実行中のインスタンスで辞書やモデルを変更することはできません。起動した後は`split`あるいは`tokenize`いずれかの実装になります。
## 注記

* コンストラクターが必要に応じてGitHubからデフォルトの辞書をインストールします。

## aknowlegements

* http://www.tkl.iis.u-tokyo.ac.jp/~ynaga/jagger/index.ja.html

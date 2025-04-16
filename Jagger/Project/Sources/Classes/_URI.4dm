Class constructor
	
Function _encodeURI($unescaped : Text)->$escaped : Text
	
	var $shouldEscape : Boolean
	var $i; $j; $code : Integer
	var $char; $hex : Text
	var $data : Blob
	
	For ($i; 1; Length:C16($unescaped))
		$char:=Substring:C12($unescaped; $i; 1)
		$code:=Character code:C91($char)
		$shouldEscape:=False:C215
		
		Case of 
			: ($code>63) & ($code<91)  //A-Z
			: ($code>96) & ($code<123)  //a-z
			: ($code>47) & ($code<58)  //0-9
			: ($code=59)  //;
			: ($code=44)  //,
			: ($code=47)  ///
			: ($code=63)  //?
			: ($code=58)  //:
			: ($code=64)  //@
			: ($code=38)  //&
			: ($code=61)  //=
			: ($code=43)  //+
			: ($code=36)  //$
			: ($code=45)  //-
			: ($code=95)  //_
			: ($code=46)  //.
			: ($code=33)  //!
			: ($code=126)  //~
			: ($code=42)  //*
			: ($code=39)  //'
			: ($code=40)  //(
			: ($code=41)  //)
			: ($code=35)  //#
			Else 
				$shouldEscape:=True:C214
		End case 
		
		If ($shouldEscape)
			CONVERT FROM TEXT:C1011($char; "utf-8"; $data)
			For ($j; 0; BLOB size:C605($data)-1)
				$hex:=String:C10($data{$j}; "&x")
				$escaped+=("%"+Substring:C12($hex; Length:C16($hex)-1))
			End for 
		Else 
			$escaped+=$char
		End if 
		
	End for 
	
	return $escaped
	
Function _encodeURIComponent($unescaped : Text)->$escaped : Text
	
	var $shouldEscape : Boolean
	var $i; $j; $code : Integer
	var $char; $hex : Text
	var $data : Blob
	
	For ($i; 1; Length:C16($unescaped))
		$char:=Substring:C12($unescaped; $i; 1)
		$code:=Character code:C91($char)
		$shouldEscape:=False:C215
		
		Case of 
			: ($code>63) & ($code<91)  //A-Z
			: ($code>96) & ($code<123)  //a-z
			: ($code>47) & ($code<58)  //0-9
			: ($code=45)  //-
			: ($code=95)  //_
			: ($code=46)  //.
			: ($code=33)  //!
			: ($code=126)  //~
			: ($code=42)  //*
			: ($code=39)  //'
			: ($code=40)  //(
			: ($code=41)  //)
			Else 
				$shouldEscape:=True:C214
		End case 
		
		If ($shouldEscape)
			CONVERT FROM TEXT:C1011($char; "utf-8"; $data)
			For ($j; 0; BLOB size:C605($data)-1)
				$hex:=String:C10($data{$j}; "&x")
				$escaped+=("%"+Substring:C12($hex; Length:C16($hex)-1))
			End for 
		Else 
			$escaped+=$char
		End if 
		
	End for 
	
	return $escaped
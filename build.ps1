#!pwsh
$nocache = ""

if($Args.Count){
	$nocache = "nocache"
}

Invoke-Expression "python ./build.py $nocache"

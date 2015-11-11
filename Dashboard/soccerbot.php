<?php

function _exec($cmd) {
	$shell = new COM("WScript.Shell");
	$result = $shell->Run($cmd, 0, false);
	echo $cmd;
	return $result == 0 ? true : false;
}

function rebuild() {
	echo exec('call "C:/soccerbot/kill.bat"')."\n";
	echo exec('call "C:/soccerbot/update.bat"')."\n";
	echo exec('call "C:/soccerbot/build.bat"')."\n";
	_exec('C:/soccerbot/soccerbot.exe');
	//echo exec('call "C:/soccerbot/run.bat"')."\n";
	//echo exec('start /B "Soccerbot" C:/soccerbot/soccerbot.exe')."\n";
	//pclose(popen('start "bla" "C:/soccerbot/soccerbot.exe"', "r"));
}

function kill() {
	echo exec('call "C:/soccerbot/kill.bat"');
}

function shutdown() {
	echo exec('call "C:/soccerbot/shutdown.bat"');
}

if (!isset($_GET['action'])) {
	echo 'Missing action';
	
	return;
}

switch ($_GET['action']) {
	case 'rebuild':
		rebuild();
	break;

	case 'kill':
		kill();
	break;

	case 'shutdown':
		shutdown();
	break;

	default:
		echo 'Unknown action: '.$_GET['action'];
	break;
}
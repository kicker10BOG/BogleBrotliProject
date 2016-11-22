<?php

include("php-redefine-function.php");

defined("ENT_XML1") or define("ENT_XML1", 16);

libxml_use_internal_errors(true);

$teamNamesToAbbr = array(
	"bills" => "buf", "dolphins" => "mia", "patriots" => "ne", "jets" => "nyj",
	"ravens" => "bal", "bengals" => "cin", "browns" => "cle", "steelers" => "pit",
	"texans" => "hou", "colts" => "ind", "jaguars" => "jac", "titans" => "ten",
	"broncos" => "den", "chiefs" => "kc", "raiders" => "oak", "chargers" => "sd", 
	"cowboys" => "dal", "giants" => "nyg", "eagles" => "phi", "redskins" => "was",
	"bears" => "chi", "lions" => "det", "packers" => "gb", "vikings" => "min",
	"falcons" => "atl", "panthers" => "car", "saints" => "no", "buccaneers" => "tb",
	"cardinals" => "ari", "seahawks" => "sea", "49ers" => "sf", "rams" => "la", "stl" => "la"
);

$positions = array(
	"qb", "hb", "fb", "wr", "te",
	"lt", "lg", "c", "rg", "rt",
	"le", "dt", "re",
	"lolb", "mlb", "rolb", 
	"cb", "ss","fs"
);

// Send a message from the bot to the group it's registered in.
defun("sendMsg", 
function($msg){
	global $bot_token;
	global $char_limit;
	$url = "https://api.groupme.com/v3/bots/post";
	
	if (strlen($msg) > $char_limit) {
		global $iMsgText;
		$base = $_SERVER['SCRIPT_URI'];
		$scriptname = explode("/", $base);
		$scriptname = $scriptname[count($scriptname)-1];
		$base = str_replace($scriptname, "", $base);
		$msg = $base . "get.php?iMsgText=" . urlencode($iMsgText);
	}
	$body = sprintf('{"text":%s,"bot_id":"%s"}', json_encode($msg), $bot_token);
	$res = \Httpful\Request::post( $url )->sendsJson( )->body( $body )->send( );
	//echo "<br>$res\n\n<br><br>$url\n\n<br><br>$body</br>";
});

// Send an image from the bot to the group it's registered in.
function sendImgMsg( $img ){
	global $bot_token;
	$url = "https://api.groupme.com/v3/bots/post";
	$body = sprintf('{"text":"","attachments":[{"type":"image","url":%s}],"bot_id":"%s"}', json_encode($img), $bot_token);
	$res = \Httpful\Request::post( $url )->sendsJson( )->body( $body )->send( );
	//echo "<br>$res\n\n<br><br>$url\n\n<br><br>$body</br>";
}

function getTeamWeekScore($team, $week) {
	global $base_url;
	global $league;
	global $teamNamesToAbbr;
	$team = getTeam($team);
	$week = getWeek($week);
	if (!$team || !$week) {
		return;
	}
	$url = $base_url . $league . "/team/" . $team . "/schedule";
	$doc = new DOMDocument();
	$doc->loadHTMLFile($url);
	$xpath = new DOMXPath($doc);
	$scoresDiv = $doc->getElementById("scores");
	$classname = "scheduleweek";
	$query = "//*[contains(concat(' ', normalize-space(@class), ' '), ' $classname ')]";
	$scores = $xpath->query($query, $scoresDiv);
	$pattern = '/\sWK\s'. $week .'\s/i';
	//$pattern = '/\sWK\s\d+/i';
	echo "<p>$pattern</p>";
	$replacement = '';
	$found = FALSE;
	$retStr = "";
	foreach ($scores as $s) {
		//echo "<p>".stristr($s->nodeValue, "WK " . $week)."</p>";
		if (stristr($s->nodeValue, "WK " . $week)) {
			$found = TRUE;
			$weekScore = preg_replace($pattern, $replacement, $s->nodeValue);
			$a = $s->getElementsByTagName("a")->item(0);
			if ($a) {
				$a = $a->getAttribute("href");
				$weekScore .= " - $a";
				$retStr = $weekScore;
			}
			else {
				foreach ($teamNamesToAbbr as $name => $abbr) {
					if ($team == $abbr) {
						$team = ucfirst($name);
						break;
					}
				}
				$retStr = "$team - BYE";
			}
			break;
		}
	}
	if (!$retStr) {
		if ($week < 18) { // not the playoffs
			foreach ($teamNamesToAbbr as $name => $abbr) {
				if ($team == $abbr) {
					$team = ucfirst($name);
					exit;
				}
			}
			$retStr = "$team - BYE";
		} 
		elseif ($scores->length > 17) {
			foreach ($scores as $s) {
				$weekNum = "";
				preg_match("/WK (\d+)/", $s->nodeValue, $weekNum);
				$weekNum = intval($weekNum[1]);
				if ($weekNum > $week) {
					foreach ($teamNamesToAbbr as $name => $abbr) {
						if ($team == $abbr) {
							$team = ucfirst($name);
							break;
						}
					}
					$retStr = "$team - BYE";
				}
			}
		}
	}
	return $retStr;
}

function getWeek($week) {
	if (!is_numeric($week)) {
		strtolower($week);
		switch ($week) {
			case "wc": $week = 18; break;
			case "dr": $week = 19; break;
			case "cc": $week = 20; break;
			case "sb": $week = 22; break;
			default: sendMsg("That is an invalid week option."); return false;
		}
	}
	$week = intval($week);
	if ($week < 1 || $week > 22 || $week == 21) {
		sendMsg("That is an invalid week option."); 
		return false;
	}
	return $week;
}

function sendLeagueScoresForWeek($week) {
	global $teamNamesToAbbr;
	$max = 0;
	$retStr = "";
	if (!is_numeric($week)) {
		strtolower($week);
		switch ($week) {
			case "wc": $max = 8; break;
			case "dr": $max = 4; break;
			case "cc": $max = 2; break;
			case "sb": $max = 1; break;
			default: sendMsg("That is an invalid week option."); return false;
		}
	}
	if ($max) {
		$retStr = sprintf("Schedule for %s week:", strtoupper($week));
		sendMsg(sprintf("Getting schedule for %s week. Please be patient.", strtoupper($week)));
	}
	else {
		$retStr = sprintf("Schedule for week %s:", $week);
		sendMsg(sprintf("Getting schedule for week %s. Please be patient.", strtoupper($week)));
	} 
	$origRetStr = $retStr;
	$seen = array();
	$i = 0;
	foreach ($teamNamesToAbbr as $key => $team) {
		echo "<br>$key<br>";
		if (!key_exists($team, $seen)) {
			$score = getTeamWeekScore($team, $week);
			if ($score) {
				$i++;
				$score = trim(preg_replace("/\s+/", " ", $score));
				$splitScore = explode(" ", $score);
				//print_r($splitScore);
				$homeTeam = $splitScore[0];
				$awayTeam = $splitScore[5];
				$seen[$teamNamesToAbbr[strtolower($homeTeam)]] = TRUE;
				$seen[$teamNamesToAbbr[strtolower($awayTeam)]] = TRUE;
				//print_r($seen);
				$retStr .= "\n" . $score;
				print("<br>$i of $max<br>");
				if ($max && $i >= $max) {
					return;
				}
			}
		}
	}
	if ($retStr == $origRetStr) {
		$retStr .= "\nNo scores were found";
	}
	sendMsg($retStr);
}

function sendTeamWeekScore($team, $week) {
	$score = getTeamWeekScore($team, $week);
	if ($score) {
		$score = trim(preg_replace("/\s+/", " ", $score));
		sendMsg($score);
	}
}

function sendPlayerSearch($params) {
	global $search_limit;
	global $teamNamesToAbbr;
	global $positions;
	global $base_url;
	global $league;
	$name = "";
	$team = "";
	$pos = "";
	$rookie = 0;
	foreach ($params as $p) {
		if (key_exists(strtolower($p), $teamNamesToAbbr)) {
			$team = $teamNamesToAbbr[strtolower($p)];
		}
		elseif (in_array(strtolower($p), $teamNamesToAbbr)) {
			$team = strtolower($p);
		}
		elseif (in_array(strtolower($p), $positions)) {
			$pos = strtoupper($p);
		}
		elseif (strtolower($p) == "r") {
			$rookie = 1;
		}
		elseif ($name) {
			$name .= " $p";
		}
		else {
			$name = $p;
		}
	}
	$searchStr = $base_url . $league . "/players?";
	if ($name) {
		$searchStr .= "name=$name&";
	}
	if ($pos) {
		$searchStr .= "position=$pos&";
	}
	if ($team) {
		$searchStr .= "team=$team&";
	}
	if($rookie) {
		$searchStr .= "rookie=1&";
	}
	$searchStr .= "sorty_by=OVR";
	$origRetStr = $searchStr .= "\nPlayers found:";
	$retStr = $origRetStr;
	$doc = new DOMDocument();
	$doc->loadHTMLFile($searchStr);
	$xpath = new DOMXPath($doc);
	$scoresDiv = $doc->getElementById("scores");
	$classname = "tbdy1";
	$query = "//*[contains(concat(' ', normalize-space(@class), ' '), ' $classname ')]";
	$players = $xpath->query($query);
	//var_dump($players);
	$search_limit = min(array($search_limit, $players->length));
	$entries = array();
	$eovrs = array();
	foreach ($players as $p) {
		//echo "\n$i\n";
		//var_dump($p);
		$name = trim($p->getElementsByTagName("a")->item(0)->nodeValue);
		$link = trim($p->getElementsByTagName("a")->item(0)->getAttribute("href"));
		$tds = $p->getElementsByTagName("td");
		$team = trim($tds->item(1)->nodeValue);
		$pos = trim($tds->item(2)->nodeValue);
		$dev = trim($tds->item(3)->nodeValue);
		$age = trim($tds->item(4)->nodeValue);
		$height = trim($tds->item(5)->nodeValue);
		$weight = trim($tds->item(6)->nodeValue);
		$ovr = trim($tds->item(7)->nodeValue);
		$myRetStr = "\n$pos $name $team Age:$age Dev:$dev OVR:$ovr $link";
		//echo "<br>$myRetStr<br>";
		if (count($entries) > 0) {
			$inserted = FALSE;
			for ($i = 0; i < count($entries) && !$inserted; $i++) {
				if (intval($ovr) > $eovrs[$i]) {
					//echo "<p>intval($ovr) intval($eovrs[$i])</p>";
					array_splice($entries, $i, 0, $myRetStr);
					array_splice($eovrs, $i, 0, intval($ovr));
					$inserted = TRUE;
					if (count($entries) > $search_limit) {
						array_pop($entries);
						array_pop($eovrs);
					}
				}
			}
		}
		else {
			$entries[0] = $myRetStr;
			$eovrs[0] = $ovr;
		}
	}
	$retStr .= implode("", $entries);
	if ($retStr == $origRetStr) {
		$retStr .= "\nNone Found";
	}
	sendMsg($retStr);
}

function sendTeamLink($team) {
	global $base_url;
	global $league;
	$team = getTeam($team);
	if (!$team) {
		return;
	}
	$link = $base_url . $league . "/team/$team/depthchart";
	sendMsg($link);
}

function sendCurrentWeek() {
	global $base_url;
	global $league;
	$url = $base_url . $league . "/schedules";
	$doc = new DOMDocument();
	$doc->loadHTMLFile($url);
	$xpath = new DOMXPath($doc);
	$classname = "home-team";
	$query = "//*[contains(concat(' ', normalize-space(@class), ' '), ' $classname ')]";
	$row = $xpath->query($query)->item(0);
	preg_match('/WK (\d+)/', $row->nodeValue, $week);
	$week = $week[1];
	switch ($week) {
		case '18':
			$week = "Wild Card Round";
			break;
		case '19':
			$week = "Divisional Round";
			break;
		case '20':
			$week = "Conf. Championships";
			break;
		case '22':
			$week = "Super Bowl";
			break;
		default:
			break;
	}
	$retStr = "Current Week: $week $url";
	sendMsg($retStr);
}

function sendTwitchLink($options) {
	$popout = false;
	$team = "";
	$team2 = "";
	$origiTeam;
	$origiTeam;
	
	foreach($options as $opt) {
		if ($opt == "p" || $opt == "popout") {
			$popout = true;
		}
		elseif (!$team) {
			if ($opt == "list") {
				$team = $opt;
			}
			else {
				$team = getTeam($opt);
				$origiTeam = $opt;
			}
		}
		else {
			$team2 = getTeam($opt);
			$origiTeam2 = $opt;
		}
	}
	
	if (!$team) {
		return;
	}
	$msg = "";
	global $xmlArr;
	$twitchNames = $xmlArr["twitch"];
	if ($team2) {
		$multiURL = "http://multitwitch.tv/";
		$twitchName = $twitchNames[$team];
		$twitchName2 = $twitchNames[$team2];
		if (!$twitchName) {
			$msg = "No twitch name listed for " . $origiTeam;
		}
		elseif (!$twitchName2) {
			$msg = "No twitch name listed for " . $origiTeam2;
		}
		else {
			$msg = "$multiURL$twitchName/$twitchName2";
		}
	}
	else {
		if ($team == "list") {
			foreach ($twitchNames as $key => $value) {
				$msg .= "$key: $value\n";
			}
			$msg = substr($msg,0, -1);
		} 
		else { 
			$twitchURL = "https://www.twitch.tv/";
			$twitchName = $twitchNames[$team];
			if (!$twitchName) {
				$msg = "No twitch name listed for " . $origiTeam;
			}
			else {
				$msg = $twitchURL . $twitchName; 
				if ($popout) {
					$msg .= "/popout";
				}
			}
		}
	}
	sendMsg($msg);
}

function sendInfo($opt = "all") {
	global $xmlArr;
	$importantInfo = $xmlArr["info"];
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		foreach($importantInfo as $k => $v) {
			$msg .= $k . ": " . $v . " \n";
		}
		$msg = substr($msg, 0, -1);
	}
	elseif (key_exists($opt, $importantInfo)) {
		$msg = $opt . ": " . $importantInfo[$opt];
	}
	sendMsg($msg);
}

function sendRings($opt = "all") {
	global $xmlArr;
	$importantInfo = $xmlArr["rings"];
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		$names = array();
		$rings = array();
		if ($importantInfo) {
			foreach($importantInfo as $k => $v) {
				//$msg .= $k . ": " . $v . " \n";
				if (count($names) == 0) {
					$names[0] = $k;
					$rings[0] = $v;
				}
				else { // *
					for ($i = 0, $placed = false; $i < count($names) && !$placed; $i++) {
						if ($v > $rings[$i]) {
							array_splice($names, $i, 0, $k);
							array_splice($rings, $i, 0, $v);
							$placed = true;
						}
					}// */
					if (!$placed) {
						array_push($names, $k);
						array_push($rings, $v);
					}
				}
			}
			for ($i = 0; $i < count($names); $i++) {
				$msg .= $names[$i] . ": " . $rings[$i] . "\n";
			} 
			$msg = substr($msg, 0, -1);
		}
		else {
			$msg = "There are no rings";
		}
	}
	elseif (key_exists($opt, $importantInfo)) {
		$msg = $opt . ": " . $importantInfo[$opt];
		if (intval($importantInfo[$opt] > 1)) {
			$msg .= " rings";
		}
		else {
			$msg .= " ring";
		}
	}
	else {
		$msg = "$opt has no rings";
	}
	sendMsg($msg);
}

function sendImg($opt = "all") {
	global $xmlArr;
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		foreach($xmlArr["img"] as $k => $v) {
			$msg .= $k . ": " . $v . " \n";
		}
		$msg = substr($msg, 0, -1);
		sendMsg($msg);
	}
	elseif (key_exists($opt, $xmlArr["img"])) {
		sendImgMsg($xmlArr["img"][$opt]);
	}
}

function sendYoutube($opt = "all") {
	global $xmlArr;
	$importantInfo = $xmlArr["youtube"];
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		foreach($importantInfo as $k => $v) {
			$msg .= $k . ": " . $v . " \n";
		}
		$msg = substr($msg, 0, -1);
	}
	elseif (key_exists($opt, $importantInfo)) {
		$msg = $opt . ": " . $importantInfo[$opt];
	}
	sendMsg($msg);
}

function sendCustom($opt = "all") {
	global $xmlArr;
	$importantInfo = $xmlArr["custom"];
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		foreach($importantInfo as $k => $v) {
			$msg .= $k . ": " . $v . " \n";
		}
		$msg = substr($msg, 0, -1);
	}
	elseif (key_exists($opt, $importantInfo)) {
		$msg = $opt . ": " . $importantInfo[$opt];
	}
	sendMsg($msg);
}

function send8Ball() {
	global $xml;
	$index = rand(0, 19);
	$msg = "Magic 8-Ball: " . $xml->magic8ball->reply[$index];
	sendMsg($msg);
}

function sendEmoji($opt = "all") {
	global $xmlArr;
	$opt = strtolower($opt);
	$msg = "";
	if ($opt == "all") {
		foreach($xmlArr["emoji"] as $k => $v) {
			$msg .= "$k, ";
		}
		$msg = substr($msg, 0, -2);
		sendMsg($msg);
	}
	elseif (key_exists($opt, $xmlArr["emoji"])) {
		sendImgMsg(htmlspecialchars_decode($xmlArr["emoji"][$opt]), ENT_XML1, 'UTF-8');
	}
}

function doAlias($cmd, $args) {
	global $xmlArr;
	global $isAdmin;
	global $cmd_prefix;
	$cmd = $xmlArr["alias"][$cmd];
	array_splice($args, 0, 0, $cmd);
	$cmd = $cmd_prefix . implode(" ", $args);
	$base = $_SERVER['SCRIPT_URI'];
	$scriptname = explode("/", $base);
	$scriptname = $scriptname[count($scriptname)-1];
	$base = str_replace($scriptname, "", $base);
	$url = "";
	if ($isAdmin) {
		$url = $base . "adminBot.php?iMsgText=" . urlencode($cmd);
	}
	else {
		$url = $base . "mainBot.php?iMsgText=" . urlencode($cmd);
	}
	//echo "<p>$url</p>\n";
	$doc = new DOMDocument();
	$doc->loadHTMLFile($url);
	//var_dump($doc);
}

function sendHelp() {
	global $league;
	global $cmd_prefix;
	global $isAdmin;
	$helpStr = format("Command Prefix: {0}
League: {1}
Commands: 
{0}twitch team [popout] : Gets a link to the twitch for the specified team
{0}custom [key]         : Gets custom info based on specified key (e.g. hello)
{0}info [key]           : Gets info based on specified key (e.g. rules, adv, owners, draft)
{0}rings [key]          : Gets number of rings based on key (PSN of player)
{0}img [key]            : Gets info based on specified key (e.g. scalp, ring)
{0}youtube [key]        : Gets youtube info based on specified key (e.g. signman)
{0}emoji [key]          : Gets emoji based on specified key (e.g. turkey, santa)
{0}8ball				: Returns a random answer 
{0}key                  : Shorthand for {0}custom key, {0}info key, {0}img key, {0}youtube key, and {0}emoji key (e.g. {0}rules, {0}scalp)
{0}help                 : This is what was just called", $cmd_prefix, $league);
	if ($isAdmin) {
		$helpStr .= format("
ADMIN COMMANDS
{0}config [key] : Gets config info based on specified key (e.g. cmdprefix, etc)
{0}set key1 key2 [value] : Sets info, config, or twitch values
", $cmd_prefix);
	}
	sendMsg($helpStr);
	/*
{0}tl team              : Gets the DaddyLeague link for that team
{0}ps options           : Gets the DaddyLeague link for the player(s) with that name
{0}sync week            : Gets all scores for the specified week
{0}tws team week        : Gets the scoreline for the specified team at the specified week
{0}week                 : Gets current week and a link to the schedule
	*/
}

function format($format) {
    $args = func_get_args();
    $format = array_shift($args);
    
    preg_match_all('/(?=\{)\{(\d+)\}(?!\})/', $format, $matches, PREG_OFFSET_CAPTURE);
    $offset = 0;
    foreach ($matches[1] as $data) {
        $i = $data[0];
        $format = substr_replace($format, @$args[$i], $offset + $data[1] - 1, 2 + strlen($i));
        $offset += strlen(@$args[$i]) - 2 - strlen($i);
    }
    
    return $format;
}

function getTeam($team) {
	global $teamNamesToAbbr;
	strtolower($team);
	if (key_exists($team, $teamNamesToAbbr)) {
		$team = $teamNamesToAbbr[$team];
	}
	else if (!in_array($team, $teamNamesToAbbr)) {
		sendMsg("That is an invalid team option.");
		return false;
	}
	return $team;
}

function xml2array($xml) {
	$arr = array();
	foreach ($xml as $element) {
		$tag = $element->getName();
		$e = get_object_vars($element);
		if (!empty($e)) {
			$arr[$tag] = $element instanceof SimpleXMLElement ? xml2array($element) : $e;
		}
		else {
			$arr[$tag] = trim($element);
		}
	}
	return $arr;
}

?>
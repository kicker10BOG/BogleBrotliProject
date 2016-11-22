<?php


function sendConfig($option) {
	global $xmlArr;
	$msg = "";
	if (!$option || $option == "all") {
		foreach ($xmlArr["config"] as $key => $value) {
			$msg .= "$key: $value\n";
		}
		$msg = substr($msg, 0, -1);
	}
	elseif (key_exists($option, $xmlArr["coinfig"])) {
		$msg = "$option: " . $xmlArr["config"][$option];
	}
	else {
		$msg = "$option is not a cofig setting";
	}
	sendMsg($msg);
}

function setCommand($options) {
	if (!$options) {
		sendMsg("no command specified");
		return;
	}
	global $xmlArr;
	$command = $options[0];
	$options = array_slice($options, 1);
	$setWorked = false;
	if (array_key_exists($command, $xmlArr)) {
		if ($command == "config") {
			$setWorked = setConfig($options);
		}
		elseif ($command == "info") {
			$setWorked = setInfo($options);
		}
		elseif ($command == "rings") {
			$setWorked = setRings($options);
		}
		elseif ($command == "youtube") {
			$setWorked = setYoutube($options);
		}
		elseif ($command == "custom") {
			$setWorked = setCustom($options);
		}
		elseif ($command == "img") {
			$setWorked = setImg($options);
		}
		elseif ($command == "alias") {
			$setWorked = setAlias($options);
		}
		elseif ($command == "emoji") {
			$setWorked = setEmoji($options);
		}
		elseif ($command == "twitch") {
			$setWorked = setTwitch($options);
		}
	}
}

function setConfig($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->config->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->config->{strtolower($options[0])});
	}
	if ($xml->asXml($xmlFile)) {
		sendMsg("Config update successful");
	}
	else {
		sendMsg("Config update unsuccessful");
	}
}

function setInfo($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->info->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->info->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Info update successful");
	}
	else {
		sendMsg("Info update unsuccessful");
	}
}

function setRings($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->rings->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->rings->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Ring update successful");
	}
	else {
		sendMsg("Ring update unsuccessful");
	}
}

function setImg($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->img->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->img->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Img update successful");
	}
	else {
		sendMsg("Img update unsuccessful");
	}
}

function setYoutube($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->youtube->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->youtube->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Youtube info update successful");
	}
	else {
		sendMsg("Youtube info update unsuccessful");
	}
}

function setCustom($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->custom->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->custom->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Custom info update successful");
	}
	else {
		sendMsg("Custom info update unsuccessful");
	}
}

function setAlias($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->alias->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->alias->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Alias update successful");
	}
	else {
		sendMsg("Alias info update unsuccessful");
	}
}

function setEmoji($options) {
	global $xml, $xmlFile;
	if (count($options) > 1) {
		$options[1] = join(" ", array_slice($options, 1));
		$xml->emoji->{strtolower($options[0])} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	}
	else {
		unset($xml->emoji->{strtolower($options[0])});
	};
	if ($xml->asXml($xmlFile)) {
		sendMsg("Emoji update successful");
	}
	else {
		sendMsg("Emoji update unsuccessful");
	}
}

function setTwitch($options) {
	global $xml, $xmlFile;
	$team = "";
	$origiTeam;
	$team = getTeam(strtolower($options[0]));
	$origiTeam = $options[0];
	if (!$team) {
		return;
	} 
	$xml->twitch->{$team} = htmlspecialchars($options[1], ENT_XML1, 'UTF-8');
	if ($xml->asXml($xmlFile)) {
		sendMsg("Twitch update successful");
	}
	else {
		sendMsg("Twitch update unsuccessful");
	}
}
?>
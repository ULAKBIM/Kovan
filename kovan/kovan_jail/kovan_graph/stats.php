<?php
	$images = "./";
	$graph_count = 0;
	$columnCount = 3;
	
	$graphs_arr[$graph_count++] = array( "protocol-daily","graph_protocol_daily",'Most Attacked Protocol (Daily)');
	$graphs_arr[$graph_count++] = array( "srcip-daily","graph_srcip_daily",'Most Attacked Source IP address (Daily)');
	$graphs_arr[$graph_count++] = array( "destip-daily","graph_destip_daily",'Most Attacked Destination IP address (Daily)');
	$graphs_arr[$graph_count++] = array( "destport-daily","graph_destport_daily",'Most Attacked Destination Port (Daily)');
	$graphs_arr[$graph_count++] = array( "service-daily","graph_service_daily",'Most Attacked Service IP address (Daily)');
	
        $graphs_arr[$graph_count++] = array( "protocol-hourly","graph_protocol_hourly",'Most Attacked Protocol (Hourly)');
	$graphs_arr[$graph_count++] = array( "srcip-hourly","graph_srcip_hourly",'Most Attacked Source IP address (Hourly)');
	$graphs_arr[$graph_count++] = array( "destip-hourly","graph_destip_hourly",'Most Attacked Destination IP address (Hourly)');
	$graphs_arr[$graph_count++] = array( "destport-hourly","graph_destport_hourly",'Most Attacked Destination Port (Hourly)');
	$graphs_arr[$graph_count++] = array( "service-hourly","graph_service_hourly",'Most Attacked Service IP address (Hourly)');

	$files_orig = dirList($images);
	$files = array_reverse($files_orig);
	$selectBoxes = "";

	for( $i=0; $i < $graph_count ; $i++)
	{
		$selectBoxes[$i] = createSelectBox($files,$graphs_arr[$i][0],$graphs_arr[$i][1]);
	}


        $html_code = '<html><head><meta http-equiv="Content-Type" content="text/html; charset=iso-8859-9"/></head><title>KOVAN Stats</title><body>';
	
	$html_code .='<div class="title">KOVAN Stats</div>';
        $html_code .='<table BORDER=1>';

	for( $i = 0; $i < $graph_count ; $i++)
	{
		if( ($i % $columnCount) == 0 ) 
		{
        		$html_code .= '<tr>';
		}
		$definition = $graphs_arr[$i][2];	
		$html_code .= "<td>$definition $selectBoxes[$i]</td>";

		if( ($i+1 % $columnCount) == 0 ) 
		{
        		$html_code .= '</tr>';
		}

	}
        $html_code .='</table>';
	echo $html_code;


function createSelectBox($fileList,$graphPrefix,$graphName)
{

	global $images;
	$selectBoxString = "";

	$selectBoxString .= "<select>";
	$selectBoxString .= "<select>";

	$selectBoxString = "<select onchange=\"if(this.value!=''){ document.getElementById('$graphName').src='./'+this.value;}\">";

	$filteredFile = "";	
	$selectBoxString .= "<option value=\"\" default >Select</option>";

	foreach ( $fileList as $fileName)
	{
		if( strstr($fileName,$graphPrefix) != FALSE )
		{
			if($filteredFile == "")
			$filteredFile = $fileName;
			$pLen = strlen($graphPrefix);
			$seenName  = substr($fileName,$pLen,strpos($fileName,'.')-$pLen);
			$selectBoxString .= "<option value=\"$fileName\">$seenName</option>";
		}
	}

	$selectBoxString .= "</select>";
	$selectBoxString .= "<img src=\"./$filteredFile\" id=\"$graphName\">";
	return $selectBoxString;
}

function dirList ($directory) 
{

    // create an array to hold directory list
    $results = array();

    // create a handler for the directory
    $handler = opendir($directory);

    // keep going until all files in directory have been read
    while ($file = readdir($handler)) {
        // if $file isn't this directory or its parent, 
        // add it to the results array
        if ($file != '.' && $file != '..')
            $results[] = $file;
    }

    // tidy up: close the handler
    closedir($handler);

    // done!
    return $results;

}


?>

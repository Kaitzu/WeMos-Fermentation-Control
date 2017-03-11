<html>

<?php
     $temp = $_GET["temp"];
     $setTemp = $_GET["setTemp"];
     $controlStatus = $_GET["controlStatus"];
     $key = $_GET["key"];
     $filename = "./templog.db";
     $now = date('Y-m-d H:i:s');
    
    if ($key != "xxxx"){
        echo "Authorization failed!";
        exit;
    }
    
    if ($temp) {
        // Create connection
        $db_handle = new SQLite3($filename);
     
        // Query
        $query_string = "INSERT INTO temps (sensorid, timestamp, temp, tempSet, controlStatus) VALUES ('99','$now','$temp','$setTemp','$controlStatus')";
        $db_handle->exec($query_string);
        }
    
?>
    
    <head>
        <title>-</title>
    </head>
    <body>
        
    </body>
    
</html>


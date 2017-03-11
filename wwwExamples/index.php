
<html>

<?php
    
    $filename = "./templog.db";
     
     // Create connection
     $db_handle = new SQLite3($filename);
     
     // Query
     $query_string = "SELECT * FROM temps WHERE timestamp >= date('now') AND sensorid = '*' ORDER BY timestamp DESC;";
     $db_handle->exec($query_string);
     
     // Results
     $result = $db_handle->query($query_string);
?>
 <head>
<title>Fermentation Temperature</title>
 </head>
    <body>
                 <?php 
                $row = $result->fetchArray(); 
                    echo $row["sensorid"] . " ";
                    $date = new DateTime($row["timestamp"]);
                    $date = $date->format('d.m.Y H:i');
                    echo "<h1 class='uk-card-title'>Fermentation Temp " . $row["temp"] . " &#8451;</h1>";
                    echo "Controller Setting: "  . $row["tempSet"] . "&#8451; State: " . $row["controlStatus"];
                    echo "<br>Updated: "  . $date;
                
                while($row = $result->fetchArray())
                {
                    
                    echo $row["sensorid"] . " ";
                    $date = new DateTime($row["timestamp"]);
                    $date = $date->format('d.m.Y H:i');
                    $time = new DateTime($row["timestamp"]);
                    $time = $time->format('H, i, s');
                    echo " " . $time . " " . $date . " " . $row["temp"] . " ";
                }
                ?>
        
    </body>
</html>

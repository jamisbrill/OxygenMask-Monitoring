<?php

DEFINE ('DBUSER', 'Jam');
DEFINE ('DBPW', '');
DEFINE ('DBHOST', '192.168.1.133');
DEFINE ('DBNAME', 'OxygenLog');

$dbc = mysqli_connect(DBHOST,DBUSER,DBPW);
if (!$dbc) {
    die("Database connection failed: " . mysqli_error($dbc));
    exit();
}

$dbs = mysqli_select_db($dbc, DBNAME);
if (!$dbs) {
    die("Database selection failed: " . mysqli_error($dbc));
    exit();
}

$result = mysqli_query($dbc, "SHOW COLUMNS FROM Log");
$numberOfRows = mysqli_num_rows($result);
if ($numberOfRows > 0) {


$values = mysqli_query($dbc, "SELECT * FROM Log");
while ($rowr = mysqli_fetch_row($values)) {
 for ($j=0;$j<$numberOfRows;$j++) {
  $csv_output .= $rowr[$j].", ";
 }
 $csv_output .= "\n";
}

}

print $csv_output;
exit;
?>

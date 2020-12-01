<!DOCTYPE html>
<html lang=”en”>
<!--Single Page Application/Form for Gateway -->

<head>
    <meta charset="utf-8">
    <title>Fire Gateway</title>
    <link href="gateway.css" rel="stylesheet" type="text/css" />
</head>

<body>
<?php

error_reporting(E_ALL);
ini_set('display_errors', 'On');

if( isset($_POST["inputCode"]) ){
  // sudo mkdir /var/lib/fireiot
  // sudo chown www-data:www-data /var/lib/fireiot
  $db = new SQLite3("/var/lib/fireiot/stored_devices.db");

  // $db->exec("CREATE TABLE StoredDevices (id int PRIMARY KEY, address  VARCHAR(40) NOT NULL, postal_code VARCHAR(6), device_type VARCHAR(20))");
  // $db->exec('INSERT INTO StoredDevices VALUES (0, " -1 null drive", "X0X123", "microwave")');

  $stm = $db->prepare('INSERT INTO StoredDevices(address, postal_code, device_type) VALUES (?, ?, "Smoke Alarm")');
  $stm->bindParam(1, $address);
  $stm->bindParam(2, $postal_code);

  $valid = true;
  if( isset($_POST["inputCode"]) ){
    //test valid input code
  }else{
    $valid = false;
  }
  if( isset($_POST["inputCountry"], $_POST["inputProvice"], $_POST["inputCity"], $_POST["inputStreet"]) ){
    // validaiton code for address format should go here.
    $address = implode ( ", " , array($_POST["inputCountry"], $_POST["inputProvice"], $_POST["inputCity"], $_POST["inputStreet"]) );
    echo "<h3>(Debugging) submitted address: " . $address . "</h3>";
  }else{
    echo "Not all fields are filled out";
    $valid = false;
  }
  if( isset($_POST["inputZip"]) ){
    //test valid zip code
    $postal_code = $_POST["inputZip"];
  }else{
    $valid = false;
  }
  if($valid){
    $stm->execute();
  }
}
 

?>
    <form action="<?=$_SERVER['PHP_SELF']?>" method="post">
        <fieldset>
          <div class="container">
            <div class="container_inner">
              <input  class="item"
                      type="text"
                      id="inputCode"
                      name="inputCode"
                      placeholder="Device Code"
                      required>
              <input  class="item"
                      type="text"
                      name="inputCountry"
                      id="inputCountry"
                      placeholder="Country"
                      value="Canada"
                      required>
              <input  class="item"
                      type="text"
                      name="inputProvice"
                      id="inputProvice"
                      value="ON"
                      placeholder="Provice"
                      required>
              <input  class="item"
                      type="text"
                      name="inputZip"
                      id="inputZip"
                      placeholder="zip"
                      required>
              <input  class="item"
                      type="text"
                      name="inputCity"
                      id="inputCity"
                      placeholder="City"
                      required>
              <input  class="item"
                      type="text"
                      name="inputStreet"
                      id="inputStreet"
                      placeholder="Street/building/unit"
                      required>
              <input  class="item"
                      type="submit" value="Submit">
            </div> 
          </div>
        </fieldset>
    </form>

</body>

</html>
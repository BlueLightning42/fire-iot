<!DOCTYPE html>
<html lang=”en”>
<!--SA0001: I Justin Whittam-Geskes, 400089960 certify that this material is my original work. No other person's work has been used without due acknowledgement. This is a modified version of lab 8 with less functionality for editing and a quick password authentication -->


<head>
    <meta charset="utf-8">
    <title>Admin View of Database</title>
    <link href="gateway.css" rel="stylesheet" type="text/css" />
    <link href="extra_admin.css" rel="stylesheet" type="text/css" />
    <style>
    </style>
</head>

<body>
    <div id="container">
        <?php

$cookie_name = "login$5tAz4MCq";
$cookie_value = "randomkeyqUI%W38Ep";

if (isset($_POST["name"], $_POST["password"])){
    $db = new SQLite3("/var/lib/fireiot/stored_devices.db");
    $qry = $db->query("SELECT user,password FROM StoredUsers");
    $result = $qry->fetchArray(SQLITE3_NUM);
	$admin_user     =$result[0];
	$admin_password =$result[1];

	if($_POST["name"]== $admin_user && $_POST["password"] == $admin_password){
		setcookie($cookie_name, $cookie_value, time() + 60*2, "/");
		$_COOKIE[$cookie_name] = $cookie_value;
	}

}

$authenticated = false;
if(isset($_COOKIE[$cookie_name])){
	$authenticated = $_COOKIE[$cookie_name] == $cookie_value;
}

if($authenticated){
    $db = new SQLite3("/var/lib/fireiot/stored_devices.db");

    $field = "id";
    if( isset($_GET["field"]) ){
        $field = $_GET["field"];
        if(!in_array($field, ["dev_name", "address", "postal_code", "id"])){
        	$field = "id";
        }
    }

    if( isset($_POST["edit"]) ){
        if( isset($_POST["dev_name"], $_POST["address"], $_POST["postal_code"]) ) {
            $stm = $db->prepare('UPDATE StoredDevices SET dev_name = ?, address = ?, postal_code = ? WHERE id = ?');
            $stm->bindParam(1, $dev_name);
            $stm->bindParam(2, $address);
            $stm->bindParam(3, $postal_code);
            $stm->bindParam(4, $id);
            $id = $_POST["edit"];
            $address = $_POST["address"];
            $dev_name = $_POST["dev_name"];
            $postal_code = $_POST["postal_code"];
            $stm->execute();
        }else{
            // should not have gotten here because fields have "required" but if html was edited or something weird was done and a value didn't get set show this error...shouldn't appear in regular use so doesn't need to be nice
            $status = "Error: malformed post request. Not all values were set";
        }
    }
    if( isset($_POST["add"]) ){
        if( isset($_POST["dev_name"], $_POST["address"], $_POST["postal_code"]) ) {
            $stm = $db->prepare('INSERT INTO StoredDevices(dev_name, address, postal_code, device_type) VALUES (?, ?, ?, "Smoke Alarm")');
            $stm->bindParam(1, $dev_name);
            $stm->bindParam(2, $address);
            $stm->bindParam(3, $postal_code);
            $address = $_POST["address"];
            $dev_name = $_POST["dev_name"];
            $postal_code = $_POST["postal_code"];
            $stm->execute();
        }else{
            // should not have gotten here because fields have "required" but if html was edited or something weird was done and a value didn't get set show this error...shouldn't appear in regular use so doesn't need to be nice
            $status = "Error: malformed post request. Not all values were set";
        }
    }
    if( isset($_POST["delete"]) ){
        $stm = $db->prepare("DELETE FROM StoredDevices WHERE id = ?");
        $stm->bindParam(1, $id);
        $id = $_POST["delete"];
        $stm->execute();
    }

    $old_values = array("TBD","","","");
    $act = "add";
    if( isset($_GET["id"]) ){
        $id = $_GET["id"];
        if( isset($_GET["act"]) ){
            $act = $_GET["act"];
            if($act == "edit" || $act == "delete"){
                $stm = $db->prepare("SELECT id, dev_name, address, postal_code FROM StoredDevices WHERE id = ?");
                $stm->bindParam(1, $id);
                $result = $stm->execute();
                $old_values = $result->fetchArray(SQLITE3_NUM);
            }else{
                $status = "Error: id set but neither edit nor delete set";
            }
        }
    }

    $result = $db->query("SELECT id, dev_name, address, postal_code FROM StoredDevices ORDER BY $field");

    echo "<h2>Stored Fire devices database</h2>";

    if( isset($status) ){
        echo "<div class ='status'>$status</div>";
    }
    $isdelete = false;
    echo "
    <form action='".$_SERVER['PHP_SELF']."' method='POST'>
        <fieldset id='display'>";
    if($act == "delete"){
        $isdelete = true;
        echo "<legend>Do you really want to delete this row?</legend>";
    }elseif($act == "edit"){
        echo "<legend>Edit device data</legend>";
    }elseif($act == "add"){
        echo "<legend>Add device data</legend>";
    }else{
        echo "<legend>...Please reload idk what act this is</legend>";
    }



    $name = $old_values[1];
    echo "
    <label for='dev_name'>Device Name</label>
    <input type='text' name='dev_name' maxlength='25' value='$name' ".($isdelete ? "class='ROM'" : "required")."><br>
    ";
    $address = $old_values[2];
    echo "
    <label for='address'>Address</label>
    <input type='text' name='address' maxlength='25' value='$address' ".($isdelete ? "class='ROM'" : "required").">
    ";
    $postal_code = $old_values[3];
    echo "
    <label for='postal_code'>postal code</label>
    <input type='text' name='postal_code' maxlength='25' value='$postal_code' ".($isdelete ? "class='ROM'" : "required")."><br>
    ";
    $id = $old_values[0];
    if($id !== "TBD"){
        echo"<br>
        <label for='$act'>ID</label>
        <input type='text' name='$act' readonly required class='ROM' value='$id'  ".($isdelete ? "class='ROM'" : "").">
        <a href='".$_SERVER["PHP_SELF"]."'>Cancel $act</a>";
    }else{
        echo" <input type='hidden' name='add' value='new'> ";
    }
    echo "<br>
    <input name='submit' type='submit' value='".(($id == "TBD") ? "Add New Reading" : ( $isdelete ? "Delete Reading" : "Update Reading" ))."'><br />";
    echo "
        </fieldset>
        </form>
        <table class='members'>
            <tr>
                <th></th>
                <th><a href='?act=sortby&amp;field=id'>ID</a></th>
                <th><a href='?act=sortby&amp;field=dev_name'>Device Name</a></th>
                <th><a href='?act=sortby&amp;field=address'>Address</a></th>
                <th><a href='?act=sortby&amp;field=postal_code'>Postal Code</a></th>
            </tr>
        ";
    while ($row = $result->fetchArray(SQLITE3_NUM)) {
        echo "\t<tr>\n\t\t<th><a href='?act=delete&amp;id=$row[0]'>D</a> <a href='?act=edit&amp;id=$row[0]'>E</a></th>\n";
        foreach ($row as $cell){
            echo "\t\t<td> $cell </td>\n";
        }
        echo "\t</tr>";
    }
    echo '</table>
        <p></p>
        <h4>&copy; Justin Whittam Geskes/ 2020-12-07 </h3>
    </div>
</body>';
}else{ // authentication
	echo '
	<form action="'.($_SERVER['PHP_SELF']).'" method="post">
        <fieldset>
          <div class="container">
            <div class="container_inner">
              <input  class="item"
                      type="text"
                      id="name"
                      name="name"
                      placeholder="name"
                      maxlength="25"
                      required>
              <input  class="item"
                      type="password"
                      name="password"
                      id="password"
                      placeholder="password"
                      maxlength="25"
                      required>
              <input  class="item"
                      type="submit" value="Submit">
            </div>
          </div>
        </fieldset>
    </form>';
}
?>
</html>

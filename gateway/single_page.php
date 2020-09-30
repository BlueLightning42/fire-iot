<!DOCTYPE html>
<html lang=”en”>
<!--Single Page Application/Form for Gateway -->

<head>
    <meta charset="utf-8">
    <title>Fire Gateway</title>
    <link href="style.css" rel="stylesheet" type="text/css" />
</head>

<body>
    <form action="<?=$_SERVER['PHP_SELF']?>" method="post">
        <fieldset>
          <div class="container">
            <div class="container_inner">
              <input  class="item"
                      type="text"
                      id="inputCode"
                      name="inputCode"
                      placeholder="Device Code">
              <input  class="item"
                      type="country"
                      name="inputCountry"
                      id="inputCountry"
                      placeholder="Country">
              <input  class="item"
                      type="province"
                      name="inputProvice"
                      id="inputProvice"
                      placeholder="Provice">
              <input  class="item"
                      type="zip"
                      name="inputZip"
                      id="inputZip"
                      placeholder="zip">
              <input  class="item"
                      type="street"
                      id="inputStreet"
                      placeholder="Street">
              <input  class="item"
                      type="submit" value="Submit">
            </div> 
          </div>
        </fieldset>
    </form>
</body>

</html>
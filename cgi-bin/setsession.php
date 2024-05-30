<?php

    // Set the session cookie to expire in one week
    ini_set('session.cookie_lifetime', 60 * 60 * 24 * 7);

    session_start();

    // Set a session variable
    $_SESSION["example"] = "Hello, World!";
?>
<!DOCTYPE html>
<html>
<head>
    <title>Session Cookie Example</title>
</head>
<body>
    <h1>Session Cookie Example</h1>
    <?php
        if (isset($_SESSION["example"])) {
            echo "<p>Session variable 'example' is set to: " . $_SESSION["example"] . "</p>";
            echo "<p><a href='readsession.php'>Go to readsession.php</a></p>";
        } else {
            echo "<p>Session variable 'example' is not set.</p>";
        }
    ?>
</body>
</html>
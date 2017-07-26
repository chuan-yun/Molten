<?php

function mysqli_exec($db, $sql) {
    $true_db = $db;

    /* mysqli connect */
    $mysql = mysqli_connect($true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
    mysqli_query($mysql, $sql);
    $mysqli_stmt = mysqli_prepare($mysql, $sql);
    mysqli_stmt_execute($mysqli_stmt);
    if ($mysqli_stmt) {
        $mysqli_stmt->fetch();
        $mysqli_stmt->close();
    }
    
    /* mysqli_stmt(execute) */
    $mysql = new mysqli($true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
    $mysql->query($sql);
    $stamt = $mysql->prepare($sql);
    if ($stamt) {
        $stamt->execute();
        $stamt->fetch();
        $stamt->close();
    }
    
    /* mysqli_stmt_prepare */
    $mysql = new mysqli($true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
    $stmt = $mysql->stmt_init();
    $result = $stmt->prepare($sql);
    if ($result) {
        $stmt->execute();
        $stmt->fetch();
    }

    /* mysqli stmt produral */
    $mysql = new mysqli($true_db['host'], $true_db['user'], $true_db['passwd'], $true_db['db']);
    $stmt = $mysql->stmt_init();
    $result = mysqli_stmt_prepare($stmt, $sql);
    if ($result) {
        mysqli_stmt_execute($stmt);
        $stmt->fetch();
        $stmt->close();
    }
}

?>

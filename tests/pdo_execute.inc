<?php

function exec_normal($db) {

    /* config */
    $dsn = $db['dsn'];
    $user = $db['user'];
    $pw = $db['passwd'];

    /* execute */
    $dbh = new PDO($dsn, $user, $pw);
    $dbh->exec("select * from configs");

    /* prepare */ 
    $dbh = new PDO($dsn, $user, $pw);
    $stat = $dbh->prepare("select * from configs");
    $result = $stat->execute();
    $stat->fetchAll();
    $stat->closeCursor();
}

function exec_error($db) {

    /* config */
    $dsn = $db['dsn'];
    $user = $db['user'];
    $pw = $db['passwd'];
    
    /* execute error sql */
    $dbh = new PDO($dsn, $user, $pw);
    $stat = $dbh->query("select * from table_not_exist3");
    if ($stat) {
        $stat->fetchAll();
        $stat->closeCursor();
    }
    $dbh->exec("select * from table_not_exist");
    $stat = $dbh->prepare("select * from table_not_exist2");
    $result = $stat->execute();
    $stat->fetchAll();
    
    /* execute error use */
    $dbh = new PDO($dsn, $user, $pw);
    $dbh->query(12121212);
    $dbh->exec(12121.1212);
    $stat = $dbh->prepare($dbh);
    if ($stat) {
      $result = $stat->execute();
    }

}

function exec_exception($db) {
    /* config */
    $dsn = $db['dsn'];
    $user = $db['user'];
    $pw = $db['passwd'];

    /* get execption */
    $dbh = new PDO($dsn, $user, $pw);	
    $dbh->setAttribute(PDO::ATTR_ERRMODE, PDO::ERRMODE_EXCEPTION);
    $dbh->beginTransaction();
    $dbh->exec("insert into configs(`key`, `value`)  values(1, 2)");
    $dbh->commit();
}

?>


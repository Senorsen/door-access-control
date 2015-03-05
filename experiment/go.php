#!/usr/bin/php
<?php
file_put_contents('text.txt', $argv[1]);
system('./parse.php');
system('./send');


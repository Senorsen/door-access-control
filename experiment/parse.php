#!/usr/bin/php
<?php
$text = file_get_contents('text.txt');
$bytes_arr = explode(':', $text);
$bin = '';
foreach ($bytes_arr as $value) {
    $bin .= chr(hexdec($value));
}
file_put_contents('data', $bin);


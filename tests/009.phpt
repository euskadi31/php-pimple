--TEST--
Test Offset Get Honors Null Values
--FILE--
<?php

$pimple = new Pimple();
$pimple['foo'] = null;
assert(is_null($pimple['foo']));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
--TEST--
Test Raw Honors Null Values
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

$pimple = new Pimple();
$pimple['foo'] = null;

assert(is_null($pimple->raw('foo')));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
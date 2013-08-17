--TEST--
Test Global Function Name As Parameter Value
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

$pimple = new Pimple();
$pimple['global_function'] = 'strlen';
assert($pimple['global_function'] == 'strlen');

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
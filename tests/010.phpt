--TEST--
Test Offset Get Honors Null Values
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

class Service {}

$pimple = new Pimple();
$pimple['param'] = 'value';
$pimple['service'] = function () {
    return new Service();
};

unset($pimple['param'], $pimple['service']);

assert(!isset($pimple['param']));
assert(!isset($pimple['service']));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
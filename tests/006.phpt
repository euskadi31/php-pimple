--TEST--
Test Isset
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

$pimple['null'] = null;

assert(isset($pimple['param']));
assert(isset($pimple['service']));
assert(isset($pimple['null']));
assert(!isset($pimple['non_existent']));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
--TEST--
Test Raw
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

$pimple = new Pimple();
$pimple['service'] = $definition = function() {
    return 'foo';
};

assert($definition === $pimple->raw('service'));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
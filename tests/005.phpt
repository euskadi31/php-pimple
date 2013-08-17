--TEST--
Test Should Pass Container As Parameter
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

class Service {
    
}

$pimple = new Pimple();

$pimple['service'] = function() {
    return new Service();
};

$pimple['container'] = function($container) {
    return $container;
};

assert($pimple !== $pimple['service']);

assert($pimple === $pimple['container']);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
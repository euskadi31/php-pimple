--TEST--
Test Services Should Be Different
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

$serviceOne = $pimple['service'];

assert(($serviceOne instanceof Service));

$serviceTwo = $pimple['service'];

assert(($serviceTwo instanceof Service));

assert($serviceOne !== $serviceTwo);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
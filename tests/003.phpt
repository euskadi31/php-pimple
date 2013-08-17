--TEST--
Test With Closure
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

assert(($pimple['service'] instanceof Service));

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
--TEST--
Test With Closure
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
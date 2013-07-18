--TEST--
Test Raw
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
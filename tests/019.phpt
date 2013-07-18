--TEST--
Test Extend Validates Key Yields Object Definition
--FILE--
<?php

try {
    $pimple = new Pimple();
    $pimple['foo'] = 123;
    $pimple->extend('foo', function() {});
} catch (InvalidArgumentException $e) {
    assert($e->getMessage() == 'Identifier "foo" does not contain an object definition.');
    echo "OK" . PHP_EOL;
}

echo "Done" . PHP_EOL;
?>
--EXPECT--
OK
Done
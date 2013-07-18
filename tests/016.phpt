--TEST--
Test Raw Validates Key Is Present
--FILE--
<?php

try {
    $pimple = new Pimple();
    $pimple->raw('foo');
} catch (InvalidArgumentException $e) {
    assert($e->getMessage() == 'Identifier "foo" is not defined.');
    echo "OK" . PHP_EOL;
}

echo "Done" . PHP_EOL;
?>
--EXPECT--
OK
Done
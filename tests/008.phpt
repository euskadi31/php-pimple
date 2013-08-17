--TEST--
Test Offset Get Validates Key Is Present
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php

try {
    $pimple = new Pimple();
    echo $pimple['foo'];
} catch (InvalidArgumentException $e) {
    assert($e->getMessage() == 'Identifier "foo" is not defined.');
    echo "OK" . PHP_EOL;
}

echo "Done" . PHP_EOL;
?>
--EXPECT--
OK
Done
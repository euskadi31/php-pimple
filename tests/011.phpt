--TEST--
Test Share
--SKIPIF--
<?php die("Skip: not implemented.");
--FILE--
<?php

class Service {}
$pimple = new Pimple();
$pimple['shared_service'] = $pimple->share(function() {
    return new Service();
});

$serviceOne = $pimple['shared_service'];
assert($serviceOne instanceOf Service);

$serviceTwo = $pimple['shared_service'];
assert($serviceTwo instanceOf Service);

assert($serviceOne === $serviceTwo);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
--TEST--
Test Extend
--SKIPIF--
<?php die("Skip: not implemented.");
--FILE--
<?php

class Service {}

$pimple = new Pimple();
$pimple['shared_service'] = $pimple->share(function() {
    return new Service();
});

$value = 12345;

$pimple->extend('shared_service', function($sharedService) use ($value) {
    $sharedService->value = $value;

    return $sharedService;
});

$serviceOne = $pimple['shared_service'];
assert($serviceOne instanceOf Service);
assert($serviceOne->value == $value);

$serviceTwo = $pimple['shared_service'];
assert($serviceTwo instanceOf Service);
assert($serviceOne->value == $value);

assert($serviceOne === $serviceTwo);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
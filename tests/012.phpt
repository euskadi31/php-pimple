--TEST--
Test Protect
--SKIPIF--
<?php die("Skip: not implemented.");
--FILE--
<?php

$pimple = new Pimple();
$callback = function() {
    return 'foo';
};
$pimple['protected'] = $pimple->protect($callback);

assert($callback === $pimple['protected']);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
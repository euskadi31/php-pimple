--TEST--
Test Protect
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
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
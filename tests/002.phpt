--TEST--
Test With String
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php 
$pimple = new Pimple();
$pimple['param'] = 'value';

//var_dump($pimple);

assert($pimple['param'] == 'value');

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
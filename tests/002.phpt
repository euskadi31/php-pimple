--TEST--
Test With String
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
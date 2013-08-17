--TEST--
Test Constructor Injection
--SKIPIF--
<?php if (!extension_loaded("pimple")) print "skip"; ?>
--FILE--
<?php
$params = array(
    "param" => "value"
);
$pimple = new Pimple($params);

assert($params['param'] == $pimple['param']);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
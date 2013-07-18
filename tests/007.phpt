--TEST--
Test Constructor Injection
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
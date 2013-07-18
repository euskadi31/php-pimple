--TEST--
Test Setting Non Invokable Object Should Treat It As Parameter
--FILE--
<?php

class NonInvokable {
    public function __call($a, $b)
    {
    }
}

$pimple = new Pimple();
$pimple['non_invokable'] = new NonInvokable();

assert($pimple['non_invokable'] instanceOf NonInvokable);

echo "Done" . PHP_EOL;
?>
--EXPECT--
Done
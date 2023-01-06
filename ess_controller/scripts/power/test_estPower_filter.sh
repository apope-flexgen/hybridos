rm output.csv
fSend=/usr/local/bin/fims/fims_send
fimsSet()
{
    $fSend -m set -u $1 $2 | jq
}
fimsGet()
{
    $fSend -m get -r /me -u $1 | jq
}
while IFS=, read -r t x
do
    fimsSet "/ess/status/bms" "{\"MaxBMSDischargePowerEstFiltIn\":{\"value\":$x}}"
    y=$(fimsGet "/ess/status/bms/MaxBMSDischargePowerEstFilt")
    printf "$t,$x,$y\n" >> output.csv
done < EstPower-data-2021-07-13.csv
sed -i 's/\r,/,/g' output.csv
# awk -F"," 'BEGIN { OFS = "," } {$3="\""2.4.0"\""; print}' EstPower-data-2021-07-13.csv > output.csv

event_set () {
    ./set_field.sh event_$event_index/year $1
    ./set_field.sh event_$event_index/month $2
    ./set_field.sh event_$event_index/day $3
    ./set_field.sh event_$event_index/mode $4
    ./set_field.sh event_$event_index/site $5
    ./set_field.sh event_$event_index/float_1 $6
    ./set_field.sh event_$event_index/duration $7
    ./set_field.sh event_$event_index/hour $8
}

#           Year  Month  Day  Mode  Site  Float_1  Duration  Hour
event_index=0
event_set   2025  2      21    1     0     90      96        19

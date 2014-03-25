    n = -1
    total = 0
    discarded = 0

    hb = khmer.new_hashbits(ht.ksize(),1,1)

    for n, record in enumerate(screed.open(
        input_filename))

    hb.consume(record.sequence)
        #how can i get the counts of a batch?
        if get_ratio(record.sequence,K,hb) > .50
            discarded += 1
        else
            ht.consume(record.sequence)
            #load kmers of count > C into hashbits
            #(TODO add C & hb as args)
            #find all kmers of K count (in ht) above C, load into hb

        if n > 0 and n % 100000 == 0:
            print '... kept {kept} of {total} or {perc:2}%'.format(
                kept=total - discarded, total=total,
                perc=int(100. - discarded / float(total) * 100.))
            print '... in file', input_filename

            if report_fp:
                print>>report_fp, total, total - discarded, \
                    1. - (discarded / float(total))
                report_fp.flush()

                hb.HT_SIZE += random.randint(-50,50)               #added
                        total += batch_size
                        #-----------------------------    
                        hb = khmer.new_hashbits(ht.ksize(),1,1)
                        assert hb
                        if not hb:
                            raise AssertionError("hashbits is not so much fun right now")

                        for n, record in enumerate(screed.open(
                            input_filename))

                        hb.consume(record.sequence)
                        #how can i get the counts of a batch?
                        ###kmers = get_counts(batch)
                        #from there i need to count the number of 0's vs 1's (present,
                        #not_present)
                        if(present > not_present)
                        discarded += 1
                    else
                        ht.consume(batch)
                        #load kmers of count > C into hashbits
                        #(TODO add C & hb as args)
                        for 
                        ###kmers.consume(above_C)

                        HT_SIZE += random.randint(-50,50)               #added

        if n> 100000 and n % 10000 == 0

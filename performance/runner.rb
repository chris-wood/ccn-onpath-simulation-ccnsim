count = 5 # 100
sizes = [128] #, 256, 512, 1024, 2048, 4096]
ks = [2] #, 3, 4, 5, 6]
prog = ARGV[0]

times = {}

sizes.each {|s|
    ks.each {|k|
        times[[s, k]] = []
        (1..count).each {|i|
            result = `#{prog} #{s} #{k}`
            times[[s, k]] << result.to_f
        }
    }
}

puts times

if [ -n "$1" ]
then
  if [ $1 = "-d" ] || [ $1 = "d" ]; then
      rm -rf ./diagram/*
  fi
else
  rm ./diagram/*
  rm *.pkl
  rm *.txt
  rm *.log
  rm *.gv
  rm -rf infile
  rm -rf outfile
fi

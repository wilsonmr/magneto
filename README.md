# ising2d

A fast parallel C++ program for various computations of the 2D [Ising model](http://en.wikipedia.org/wiki/Ising_model). Example uses:

```
make
ising -L=60 -TSteps=9 -measure=energy -o=ising_e
ising -L=60 -TSteps=12 -measure=mag -o=ising_m
```

It's using OpenMP for the parallel work and some parts of C++11. No external libraries needed.

There is already a lot of Ising code out there, but most are sub-optimal for various reasons. Among those are slow performance, unreadable code, nonexistant documentation, incorrect normalization of the physics or missing calculations. I hope this program is slightly better.

## Usage
The grid size can be set with `-L=32`.

The temperatures are controled with three parameters: Min and max temperature and the number of steps. So `-TMin=0.0 -TMax=3.0 -TSteps=4` would do calculate for T=0.0, T=1.0, T=2.0, T=3.0. It's equivalent to numpy's `np.linspace(0.0, 3.0, num=4, endpoint=True)`.

The grid is randomly initialized with 1 or -1 values. The equilibration is done with the Swendsen-Wang algorithm. From there, a number of different computations can be done. Also see the [IPython Notebook](http://nbviewer.ipython.org/github/s9w/ising2d/blob/master/usage.ipynb) for examples. The number of threads can be controlled with `-threads=4`.

### States
To output states, use `-measure=states`. A csv file with 0 and 1 corresponding to the spin states will be written.

Example: generate file with `ising -TMin=2.5 -TSteps=1 -L=80 -measure=states -o=states`. To plot with matplotlib: `plt.imshow(np.loadtxt("states0.txt",  delimiter=","), cmap=plt.cm.Greys, interpolation ="none")`. Result:

![Example state](http://i.imgur.com/xXkFltH.png).

### Energy
`-measure=energy` measures the energy of the system given by the hamiltonian. It's averaged over `avgN` different systems.

### Magnetization
`-measure=mag` measures the absolute value of the magnetization. It's averaged over `avgN` different systems.

### Heat capacity
`-measure=cv` measures the specific heat capacity from energy variations. Calculated from `avgN` different systems.

### Magnetic Susceptibility
`-measure=cv` measures the specific heat capacity from the variation of the magnetization. Calculated from `avgN` different systems.

## Options
Overview of all parameters and their defaults:
- `-L=30`: The size of the grid. Produces grid of size L*L
- `-TMin=0.1`: Minimal temperature
- `-TMin=4.53`: Maximal temperature. Approx 2*T_critical
- `-TSteps=10`: Number of temperature steps. Divides the range into 10 pieces
- `-avgN=500`: The size of the grid. Produces grid of size L*L
- `-o=out`: Filename base for the results. `.txt` gets appended 
- `-threads=2`: Number of parallel threads
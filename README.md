# Hausarbeit
## Modelldefinition PHALS
# Natation

|Indizes und Mengen          |                                                                                  |
| --------------------------------------------------- | ------------------------------------------------------- |
|$i,j \ \ \in \mathcal{I}$                            | Menge der Coils                                         |       
|$k \in \mathcal{K}$  				      | Menge der parallelen Linien                             |
|$m \in \mathcal{M_{ik}},~n \in \mathcal{M_{jk}}$    | Menge der möglichen Modi für Coil i und j auf Linie k   |


|Parameter                                            |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
|$p_{ikm}$                                            | Bearbeitungsdauer von Coil i auf der Linie k in Modus m |
|$\alpha$                                             | Maximale Anzahl an verspäteten Coils                    |
|$d_i$                                                | Fälligkeitsdatum von Coil i                             |
|$c_{ijkmn}$                                          | Kosten für einen Stringer zwischen Coil i in Modus m und Coil |
|$t_{ijkm}$                                          |Bearbeitungsdauer von einem Stringer zwischen Coil i in Modus |

|Entscheidungsparameter                               |                                                         |
| --------------------------------------------------- | ------------------------------------------------------- |
| $X_{ijkmn} \in \{0,1\}$                             |  1, wenn Coil i in Modus m direkt vor Coil j in Modus n auf der Linie k produziert wird, 0 sonst |
| $Z_{i} \in \{0,1\}$                                 | 1, wenn Coil i Verspätung hat, 0 sonst                  |
| $S_i \geq 0$                                        | Startzeit der Bearbeitung von Coil i                    |

## Kompaktes Modell
```math
Min \sum_{i \in \mathcal{I}} \sum_{j \in \mathcal{I}} \sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}} X_{ijkmn} \cdot  c_{ijkmn}\\
		
unter den Nebenbedingungen:\\
		
\sum_{j=1}^{I+1} \sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}}  X_{ijkmn} = 1~~~~~ \forall   i \in \mathcal{I} \\
\sum_{j\in \mathcal{I}} \sum_{n \in \mathcal{M}_{jk}}  X_{0jk0n} =1 ~~~~~ \forall   k \in \mathcal{K} \\
\sum_{i\in \mathcal{I}} \sum_{m \in \mathcal{M}_{ik}}  X_{i(I+1)km0} =1 ~~~~~ \forall   k \in \mathcal{K} \\
\sum_{i=0}^{I} \sum_{m \in \mathcal{M}_{ik}}  X_{ijkmn} - \sum_{i=1}^{I+1} \sum_{m \in \mathcal{M}_{ik}} X_{jiknm}= 0 ~~~~~ \forall   k \in \mathcal{K},~ j \in \mathcal{J},~n \in \mathcal{M}_{jk}  \\
S_i+ \sum_{j=1}^{I+1} \sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}}  p_{ikm} \cdot X_{ijkmn} \leq d_i+Z_i \cdot M~~~~~\forall   i \in \mathcal{I} \\
S_i+ \sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}}  (p_{ikm}+t_{ijkmn}) \cdot X_{ijkmn} \leq S_j+M \cdot (1-\sum_{k \in \mathcal{K}} \sum_{m \in \mathcal{M}_{ik}} \sum_{n \in \mathcal{M}_{jk}} X_{ijkmn})~~~~~\forall   i,j \in \mathcal{I} \\
\sum_{i\in \mathcal{I}} Z_i \leq \alpha \\

```



## Rendered pdf
# ba-thesis-paper
Rendered paper pdf can be accessed [here](https://gitlab.uni-hannover.de/christopher.glanderluh/or-ii-final-project/-/jobs/artifacts/main/raw/Assignment/Hausarbeit/Hausarbeit.pdf?job=paper).
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

## Master

		
unter den Nebenbedingungen:
\begin{align}
    S_{ik} + 
        \sum_{j \in \setCoilsEnd} 
        \sum_{m \in \setModes{i}{k}}
        \sum_{n \in \setModes{j}{k}}
            \processingt{i}{k}{m} \cdot \dX{i}{j}{k}{m}{n}
        \leq \due{i} + Z_{ik}\cdot M
                                            && \forall i \in \setCoils, k \in \setLines
\end{align}


\begin{align}
    S_{ik} &+ 
        \sum_{m \in \setModes{i}{k}}
        \sum_{n \in \setModes{j}{k}}
            (\processingt{i}{k}{m} + \setupt{i}{j}{k}{m}{n}) \cdot \dX{i}{j}{k}{m}{n} \nonumber \\ 
        &\leq S_{ik}+ M\cdot \left(1- 
                                \sum_{m \in \setModes{i}{k}}
                                \sum_{n \in \setModes{j}{k}}
                                \dX{i}{j}{k}{m}{n}
                            \right)
                            &&\forall i, j \in \setCoils, k \in \setLines
\end{align}

## Pricer

unter den Nebenbedingungen:
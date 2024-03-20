function addRow2(ele) {

  let iRow = ele.closest('tr').sectionRowIndex;

  let table = document.getElementById("ExTable2");   
  let row = table.insertRow(iRow+1);
  row.innerHTML = '<td><input type="text" name="Ex" size="5" /></td> \
                   <td><input type="text" name="Jpi" size="5"/></td> \
                   <td><input type="text" name="Orb" size="6"/></td> \
                   <td><button type="button" onclick="addRow(this)">Insert Ex</button></td> \
                   <td><button type="button" onclick="deleteRow(this)">Remove Ex</button></td>';
}

function deleteRow2(ele){
  let table = document.getElementById("ExTable2");
  let nRow = table.rows.length;
  let iRow = ele.closest('tr').sectionRowIndex;
  if ( nRow > 2){
     table.deleteRow(iRow);
  }
}

let parity;
function checkParity(){
  parity = 0;
  if( document.getElementById('pos').checked == true ) parity += 1;
  if( document.getElementById('neg').checked == true ) parity += 2;
  if( document.getElementById('unk').checked == true ) parity += 4;
  //console.log(parity);
}

checkParity();

function addStates(){
  let AZ = document.getElementById('heavyName').value;
  let maxEx = document.getElementById('maxEx').value;
  let beamJpi = document.getElementById('beam_Jpi').value;
    
  let str = 'get_nuclear_data.py?isotopes_name=' + AZ + '&maxEx='+maxEx;
  
  let table = document.getElementById("ExTable"); 
  
  const client = new XMLHttpRequest();
  
  client.addEventListener('loadstart', 
    function(e){
      document.getElementById('waiting').innerHTML = "wait....retrieving data from IAEA..";
    }
  );
  
  client.addEventListener('error', 
    function(e){
      document.getElementById('waiting').innerHTML = "Error.";
    }
  );
  
  client.addEventListener('loadend',  
    function(e){
      let result = client.responseText.split(/\r?\n/);
      
      //clear table
      let nRow = table.rows.length;
      for( let j = nRow; j > 2; j--){
        table.deleteRow(j - 2);
      }
      
      document.getElementById('waiting').innerHTML = "";
      let count = 0;      

      for( let i = 0; i < result.length; i++){
        if( i < 17 ) continue;
        if( result[i] == "</table>" ) break;
        
        let kaka = result[i].split(' ').filter(n => n);
        
        let ex  = parseFloat(kaka[3])/1000.;
        let jpi = kaka[7]?.replace('(', '')?.replace(')', '');
        console.log(ex + ", " + jpi);
        
        //check parity
        if( (((parity >> 2) & 1) != 1) && kaka[7].slice(-1) == ")" ) continue;
        if( (((parity >> 2) & 1) != 1) && jpi == "," ) continue;
        if( (((parity) & 1) != 1) && jpi.slice(-1) == "+" ) continue;
        if( (((parity >> 1) & 1) != 1) && jpi.slice(-1) == "-" ) continue;
        
        count ++;
        nRow = table.rows.length;
        let row = table.insertRow(nRow-1);
        row.innerHTML = '<td><input type="text" name="Ex"  size="5" value="' + ex.toFixed(3) + '"/></td> \
                         <td><input type="text" name="Jpi" size="5" value="' + jpi + '"/></td> \
                         <td><input type="text" name="Orb" size="6" /></td> \
                         <td><button type="button" onclick="addRow(this)">Insert Ex</button></td> \
                         <td><button type="button" onclick="deleteRow(this)">Remove Ex</button></td> \
                         <td>'+ kaka[7] +'</td>';
        
      }
      if( count == 0 ){
         document.getElementById('waiting').innerHTML = "no states found.";
         nRow = table.rows.length;
         let row = table.insertRow(nRow-1);
         row.innerHTML = '<td><input type="text" name="Ex"  size="5" value="0"/></td> \
                         <td><input type="text" name="Jpi" size="5" value="1/2+"/></td> \
                         <td><input type="text" name="Orb" size="6" value="1s1/2"/></td> \
                         <td><button type="button" onclick="addRow(this)">Insert Ex</button></td> \
                         <td><button type="button" onclick="deleteRow(this)">Remove Ex</button></td>';
      }
    }
  );

  client.open('GET', str);
  client.send();
  
}